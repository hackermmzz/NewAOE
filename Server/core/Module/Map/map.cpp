#include "map.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>

// 公共性能模块：提供 PROFILE_TRACE()，用于记录函数耗时；文档未单列维护组。
#include "Public/profile/profile.h"
// 公共序列化模块：提供 FArchiveIn 及 Serialize()/IsError()/IsEnd()；文档未单列维护组。
#include "Public/serializer/FarchiveIn.h"

// Player：玩家组负责的玩家类型；当前仅预留参数，初始对象加载要对接其创建接口。
// 本文件的 std:: 类型和函数来自 C++ 标准库，不属于项目其他模块组。
bool Map::loadMap(const std::string& mapFile, std::vector<Player>& players){
    PROFILE_TRACE();

    // 1. 打开地图文件，并定位到文件末尾。
    std::ifstream file(mapFile, std::ios::binary | std::ios::ate);
    if (!file){
        return false;
    }

    // 2. 获取文件大小，检查是否为空或超出读取器支持的长度。
    // I32/I64/U32/U64/Byte：Public/type/type.h 提供的公共基础类型，文档未单列维护组。
    const I64 byteCount = static_cast<I64>(file.tellg());
    if (byteCount <= 0
        || static_cast<U64>(byteCount) > std::numeric_limits<U32>::max() / 8){
        return false;
    }

    // 3. 回到文件开头，将完整内容读入缓冲区。
    std::vector<Byte> buffer(static_cast<std::size_t>(byteCount));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(buffer.data()),
                   static_cast<std::streamsize>(buffer.size()))){
        return false;
    }

    // 4. 按 bit 长度创建读取器，准备临时地形和重复位置标记。
    const U32 bitCount = static_cast<U32>(buffer.size() * 8);
    FArchiveIn archive(buffer.data(), bitCount);
    // BlockGrid 是 Map 自己定义的数组别名，其中的 Block 是外部基础地块类型。
    // 按文档 BaseObject 整体分工对接玩家组；旧 Block.h 不能视为新版接口已确认。
    BlockGrid loadedCells{};
    // MAP_L/MAP_U：地图格数，来源 Server/core/Public/config/Core.h，维护组未单列。
    bool visited[MAP_L][MAP_U]{};

    // 5. 逐条读取地块，检查坐标、重复位置和地形类型，存入临时地图。
    // 暂按无文件头、完整 MAP_L × MAP_U 条地块记录读取，待写入器确认。
    for (U32 i = 0; i < MAP_L * MAP_U; i++){
        Block block{};
        // blockpos/blockType/blockHeight：暂沿用旧 Block 的成员名，需基础类型负责人确认新版。
        // blockpos 的类型 IVec2 及其 x/y 成员来自 Public/vec/vec.hpp 公共向量库，维护组未单列。
        archive.Serialize(block.blockpos.x, block.blockpos.y,
                          block.blockType, block.blockHeight);
        if (archive.IsError()){
            return false;
        }

        const I32 x = block.blockpos.x;
        const I32 y = block.blockpos.y;
        if (x < 0 || x >= static_cast<I32>(MAP_L)
            || y < 0 || y >= static_cast<I32>(MAP_U)){
            return false;
        }
        if (visited[x][y]){
            return false;
        }
        // MAPPATTERN_GRASS/MAPPATTERN_OCEAN/MAPPATTERN_SHOAL：公共地形枚举。
        // 来源 Server/core/Public/var/Terrain.h，维护组未单列。
        if (block.blockType != MAPPATTERN_GRASS
            && block.blockType != MAPPATTERN_OCEAN
            && block.blockType != MAPPATTERN_SHOAL){
            return false;
        }

        visited[x][y] = true;
        loadedCells[x][y] = block;
    }

    // 6. 检查地形段之后是否还有当前不支持的数据。
    // TODO: Player 和对象创建接口补齐后，使用 players 创建初始玩家对象，并接入动物和资源加载。
    if (!archive.IsEnd()){
        return false;
    }

    // 7. 全部校验通过后替换正式地图，并清空旧障碍图。
    for (U32 x = 0; x < MAP_L; x++){
        for (U32 y = 0; y < MAP_U; y++){
            cells[x][y] = loadedCells[x][y];
            barrierMap[x][y] = 0;
        }
    }

    // 8. 使旧地图对象的所有 SafePtr/WeakPtr 引用失效，再释放 Map 持有的引用。
    // destroy() 来自公共 SafePtr；仅 clear() 不会使其他位置持有的对象引用失效。
    for (auto& animal : animals){
        animal.destroy();
    }
    for (auto& resource : staticResources){
        resource.destroy();
    }
    animals.clear();
    staticResources.clear();
    return true;
}

// Animal：BaseObject 动物基础类，按文档对接玩家组；Map 负责持有和管理动物集合。
// SafePtr：BaseObject/SafePointer.hpp 的公共指针设施，具体维护人未单列。
const std::vector<SafePtr<Animal>>& Map::getAnimals() const{
    return animals;
}

// StaticRes：旧 StaticResource.h 中的类名，对应文档 StaticResource；新版名称由基础类型负责人确认。
const std::vector<SafePtr<StaticRes>>& Map::getStaticResource() const{
    return staticResources;
}

// getglobalNum()：Object.h 现有的公开编号声明，按 BaseObject 整体分工对接玩家组。
// 当前 Object.cpp 尚未实现此函数，且 SN 是 U64、此接口返回 I32，需负责人确认编号范围。
// 这里仅按现有 I32 接口编写查找逻辑，不直接访问 SN，也不代替其他组实现编号转换。
// destroy() 是公共 SafePtr 的逻辑销毁接口；animals/staticResources 容器本身由 Map 管理。
void Map::removeAnimal(I32 globalID){
    PROFILE_TRACE();

    for (auto it = animals.begin(); it != animals.end(); it++){
        auto& animal = *it;
        // 先确认指针有效，才能通过 Object 的编号接口读取对象。
        if (animal && animal->getglobalNum() == globalID){
            animal.destroy();
            animals.erase(it);
            return;
        }
    }
    // 编号不存在或对象已经失效时，无需重复删除。
}

void Map::removeStaticResouce(I32 globalID){
    PROFILE_TRACE();

    for (auto it = staticResources.begin(); it != staticResources.end(); it++){
        auto& resource = *it;
        if (resource && resource->getglobalNum() == globalID){
            resource.destroy();
            staticResources.erase(it);
            return;
        }
    }
}

void Map::generateBarrierMap(){
    PROFILE_TRACE();

    // 外部依赖：玩家组提供各玩家的对象集合；基础对象负责人提供 Object 的碰撞盒访问接口。
    // TODO: 接入全部玩家对象、动物及静态资源，并由 Object 公开碰撞盒类型与读取接口。
    // TODO: 统一障碍格数值和贴边占格规则后，清空 barrierMap 并按矩形/圆形重新标记。
    throw std::logic_error("Map::generateBarrierMap requires object collision-box access");
}

void Map::tick(){
    PROFILE_TRACE();

    // 内核执行模块由文档作者负责，每帧调用时清理 Map 集合中的空指针和已逻辑销毁的引用。
    // SafePtr 的有效性判断会读取共享销毁状态，不再访问已经失效的对象。
    animals.erase(std::remove_if(animals.begin(), animals.end(),
        [](const SafePtr<Animal>& animal){ return !animal; }), animals.end());
    staticResources.erase(std::remove_if(staticResources.begin(), staticResources.end(),
        [](const SafePtr<StaticRes>& resource){ return !resource; }), staticResources.end());

    // TODO: 玩家集合、Object 碰撞盒接口及刷新时序确认后，再接入 generateBarrierMap()。
}
