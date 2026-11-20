#pragma once

#include <string>
#include <vector>

#include "Server/core/BaseObject/Animal.h"
#include "Server/core/BaseObject/Block.h"
#include "Server/core/BaseObject/SafePointer.hpp"
#include "Server/core/BaseObject/StaticResource.h"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Public/var/Terrain.h"

class Player;

class Map{
public:
    using BlockGrid = Block[MAP_L][MAP_U];
    using BarrierGrid = I32[MAP_L][MAP_U];

    // 当前只支持地形段；文件非法或含尚不支持的对象段时返回 false，原地图保持不变。
    bool loadMap(const std::string& mapFile, std::vector<Player>& players);

    FORCE_INLINE I32 getBlockHeight(IVec2 blockPos) const{
        return cells[blockPos.x][blockPos.y].blockHeight;
    }

    FORCE_INLINE I32 getBlockType(IVec2 blockPos) const{
        return cells[blockPos.x][blockPos.y].blockType;
    }

    FORCE_INLINE bool isOcean(IVec2 blockPos) const{
        return getBlockType(blockPos) == MAPPATTERN_OCEAN;
    }

    FORCE_INLINE bool isBeach(IVec2 blockPos) const{
        if (isOcean(blockPos)) return false;

        for (I32 dx = -1; dx <= 1; dx++){
            for (I32 dy = -1; dy <= 1; dy++){
                if (dx == 0 && dy == 0) continue;

                const IVec2 neighbor{blockPos.x + dx, blockPos.y + dy};
                if (neighbor.x >= 0 && neighbor.x < static_cast<I32>(MAP_L)
                    && neighbor.y >= 0 && neighbor.y < static_cast<I32>(MAP_U)
                    && isOcean(neighbor)){
                    return true;
                }
            }
        }
        return false;
    }

    const std::vector<SafePtr<Animal>>& getAnimals() const;
    const std::vector<SafePtr<StaticRes>>& getStaticResource() const;
    // 按现有 Object 编号接口查找；找到后逻辑销毁并移除，不存在时不作处理。
    void removeAnimal(I32 globalID);
    void removeStaticResouce(I32 globalID);

    void generateBarrierMap();

    FORCE_INLINE const BarrierGrid& getBarrierMap() const{
        return barrierMap;
    }

    FORCE_INLINE const BlockGrid& getMap() const{
        return cells;
    }

    // 每帧清理动物、资源集合中的无效 SafePtr，不负责对象自身的 tick 调度。
    void tick();

private:
    BlockGrid cells{};
    BarrierGrid barrierMap{};
    std::vector<SafePtr<Animal>> animals;
    std::vector<SafePtr<StaticRes>> staticResources;
};
