#include "map.h"

#include <fstream>
#include <limits>
#include <stdexcept>

#include "Public/profile/profile.h"
#include "Public/serializer/FarchiveIn.h"

bool Map::loadMap(const std::string& mapFile, std::vector<Player>&){
    PROFILE_TRACE();

    // 1. 打开地图文件，并定位到文件末尾。
    std::ifstream file(mapFile, std::ios::binary | std::ios::ate);
    if (!file){
        return false;
    }

    // 2. 获取文件大小，检查是否为空或超出读取器支持的长度。
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
    BlockGrid loadedCells{};
    bool visited[MAP_L][MAP_U]{};

    // 5. 逐条读取地块，检查坐标、重复位置和地形类型，存入临时地图。
    // 暂按无文件头、完整 MAP_L × MAP_U 条地块记录读取，待写入器确认。
    for (U32 i = 0; i < MAP_L * MAP_U; ++i){
        Block block{};
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
        if (block.blockType != MAPPATTERN_GRASS
            && block.blockType != MAPPATTERN_OCEAN
            && block.blockType != MAPPATTERN_SHOAL){
            return false;
        }

        visited[x][y] = true;
        loadedCells[x][y] = block;
    }

    // 6. 检查地形段之后是否还有当前不支持的数据。
    // TODO: Player 和对象创建接口补齐后，接入对象段解析。
    if (!archive.IsEnd()){
        throw std::logic_error("Map::loadMap: object records or trailing data are not supported yet");
    }

    // 7. 全部校验通过后替换正式地图，并清空旧障碍图。
    for (U32 x = 0; x < MAP_L; ++x){
        for (U32 y = 0; y < MAP_U; ++y){
            cells[x][y] = loadedCells[x][y];
            barrierMap[x][y] = 0;
        }
    }

    // 8. 清空旧地图的动物和资源集合，完成本次地形加载。
    animals.clear();
    staticResources.clear();
    return true;
}

const std::vector<SafePtr<Animal>>& Map::getAnimals() const{
    return animals;
}

const std::vector<SafePtr<StaticRes>>& Map::getStaticResource() const{
    return staticResources;
}

void Map::removeAnimal(I32){
    // TODO: 统一并实现 Object 的编号接口后，按 SN 查找、destroy，再从 animals 移除。
    throw std::logic_error("Map::removeAnimal requires the Object ID interface");
}

void Map::removeStaticResouce(I32){
    // TODO: 统一并实现 Object 的编号接口后，按 SN 查找、destroy，再从 staticResources 移除。
    throw std::logic_error("Map::removeStaticResouce requires the Object ID interface");
}

void Map::generateBarrierMap(){
    PROFILE_TRACE();

    // TODO: 接入全部玩家对象、动物及静态资源，并由 Object 公开碰撞盒类型与读取接口。
    // TODO: 统一障碍格数值和贴边占格规则后，清空 barrierMap 并按矩形/圆形重新标记。
    throw std::logic_error("Map::generateBarrierMap requires object collision-box access");
}

void Map::tick(){
    // TODO: 与内核确定刷新时序后，在约定位置接入 generateBarrierMap()。
}
