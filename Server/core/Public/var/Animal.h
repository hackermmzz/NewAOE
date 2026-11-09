#pragma once

/*************************动物种类**********************/
enum ANIMAL {
    ANIMAL_TREE = 0,
    ANIMAL_GAZELLE,         // 瞪羚
    ANIMAL_ELEPHANT,        // 大象
    ANIMAL_LION,            // 狮子
    ANIMAL_ALLIGATOR,       // 鳄鱼
    ANIMAL_ALLIGATOR_KING,  // 鳄鱼王
};

/**************************动物友好程度*******************/
enum FriendlyType {
    FRIENDLY_NULL = 0,
    FRIENDLY_FRI,
    FRIENDLY_ENEMY,
    FRIENDLY_FENCY
};
