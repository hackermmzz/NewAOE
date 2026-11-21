# Player 模块说明

> 本文档描述 `Server/core/Module/Player/` 的职责、对象登记方式与升级机制。
> 内容与当前代码（`feature/player`，提交 `5632a79`）一致，不含推测。

---

## 1. 定位与边界

`Player` 代表「一个玩家」：它的资源、名下对象、科技引用、以及**兵种/建筑的升级进度**。

**负责：**

- 对象的创建与登记（`newXXX`）、按 SN 移除（`removeXXX`）
- 玩家四种资源的读写
- 接收转化来的敌方对象（`receiveXXX`）
- 记录本玩家的 `Development`（供自己和外部查询）
- 建筑动作完成后的**数值刷新**与**升级处理**（`notifyBuildingActionFinish`）

**不负责：**

| 事项 | 归属 |
|---|---|
| 造人时决定出生点、扣资源、排队 | 内核（内核调 `newXXX` 后才进 Player） |
| 科技是否可研究、费用、耗时 | `Development::buildingActionAble` / `buildingActionTimeCost` |
| 建造进度推进 | `Building::build` |
| 人口 / 人口上限 | 内核 |
| 每帧行为 | 内核 |

一句话：**Player 是对象的登记簿 + 升级状态的持有者**，不做决策。

---

## 2. 数据成员

```cpp
I32 civilization;                  // 文明（16 种，见 Civilization.h）
I32 teamID;                        // 团队编号
I32 playerID;                      // 玩家编号，init() 前为 -1
WeakPtr<Development> playerScience;// 本玩家的科技，只读
array<I32,4> ResourceGroup;        // 木 / 食 / 石 / 金

vector<SafePtr<Army>>             armies;
vector<SafePtr<Farmer>>           farmers;
vector<SafePtr<Building>>         buildings;
vector<SafePtr<BuildingResource>> buildingResources;   // 农田，与 buildings 分开存
vector<SafePtr<Missile>>          missiles;

I32 __OBJECTREALLYTYPE__[22];      // 升级槽位：code -> 已升到的级别
```

**为什么 `buildingResources` 单独一个容器**：资源型建筑的 `sort` 是 `SORT_BUILDING_RESOURCE`，
和普通建筑不是一类；混在 `buildings` 里会让「按 sort 选处理方式」的代码反复判断，也容易出现一个对象被两个容器各持一份。

副作用：对农田调 `removeBuilding` 不会删掉它，必须调 `removeBuildingResource`。内核删对象时要按 `sort` 选对函数。

---

## 3. 对象生命周期

### 3.1 `init`

```cpp
Player p(civilization, teamID);   // 构造函数：设文明/团队，playerID = -1，按文明给开局资源
p.init(playerID);                 // 真正分配玩家编号
```

`playerID` 在构造函数里先置 `-1`：`setBelong` 只在参数 `>= 0` 时才写入，
所以 `init()` 之前误调 `newXXX` 不会把垃圾值写进对象归属。

### 3.2 `newXXX`

五个工厂函数的流程一致：

```cpp
Army* Player::newArmy(IVec2 detailPos, I32 type, WeakPtr<Development> playerScience)
{
    rememberPlayerScience(playerScience);                       // 1. 记住科技（只认第一次）
    SafePtr<Army> a(new Army(detailPos,
                             redirectObjectType(SORT_ARMY, type), // 2. 按科技抬到最高可用级别
                             playerScience));
    a->setBelong(playerID, teamID);                             // 3. 写归属
    armies.push_back(a);                                        // 4. 入容器
    return a.get();                                             // 5. 返回裸指针给内核
}
```

- `rememberPlayerScience`：只在当前为空时写入，后续重复传入不覆盖。
- `redirectObjectType`：内核传的是「底层兵种」也能造出升级后的（见 §6）。
- 返回裸指针：对象由 Player 的容器持有，调用方不应自行 `delete`。
- **`newBuildingResource` 不经过 `redirectObjectType`**：农田没有升级链。

### 3.3 `removeXXX`

线性扫描按 `SN` 删除，找到就 `erase` 并 `return`。
`SafePtr` 被 `destroy()` 过之后 `operator->` 返回 `nullptr`，所以循环里先判 `IsNull()`；
`erase` 后迭代器失效，必须立即返回，不能继续 `++`。

单玩家对象量级在几百，线性扫描可接受（不建索引）。

### 3.4 `tick`

目前是空函数。原先用于清理容器里已 `destroy` 的空槽位，现在改由调用方保证 `removeXXX`，
`refreshAllObjectScience` / `changeObjectType` 内部都做了 `IsNull()` 判断，空槽不会崩，但会一直占位。

### 3.5 容器访问

`getArmy()` / `getFarmer()` / `getBuilding()` / `getBuildingResource()` / `getMissile()`
返回容器的**非 const 引用**，内核可以直接遍历甚至改容器内容 —— 这是出于性能考虑，
但意味着「谁改了容器」不受约束。建议内核只读，增删一律走 `newXXX` / `removeXXX`。

---

## 4. 资源接口

```cpp
I32          getResource(I32 resourceSort);              // 越界返回 0
array<I32,4> getResource();                              // 木/食/石/金
bool changeResource(I32 resourceSort, I32 changeAmount); // 不够扣返回 false 且不改
bool changeResource(I32 wood, I32 food, I32 stone, I32 gold); // 四个一起，任一不够则全部不改
```

- `ResourceGroup` 只有 4 项，而 `ProductSort`（渔获走的那套）恰好有 5 项，
  所以所有入口都做了 `[0,4)` 的范围检查 —— 传错一次就是越界读写，不会崩但会静默改坏内存。
- 四参数版本先整体校验再写入，保证不会出现「扣了木但金不够」的半成品状态。

---

## 5. 转化（祭司招降）

```cpp
bool receiveArmy(SafePtr<Army> army);
bool receiveBuilding(SafePtr<Building> building);
bool receiveFarmer(SafePtr<Farmer> farmer);
bool receiveBuildingResource(SafePtr<BuildingResource> buildingResource);
```

行为：

1. 判空、判同阵营（同队不允许转化）
2. `setBelong(playerID, teamID)` 改归属
3. `setPlayerScience(playerScience)` —— **科技改为跟随本玩家**
4. 放进对应容器

两个刻意的设计：

- **不拷贝科技快照**。早期实现是把对方的 `Development` 拷一份锁死，现在改成直接指向我方科技，招降来的单位会跟着我方研究一起变强。
- **不经过 `redirectObjectType`**。所以「我只能造骑兵，但招降了一个重骑兵」，那个重骑兵保持重骑兵，不会因为我的科技低而被降级。
- **改完科技会立刻 `refresh()`**。否则招降来的单位要等下一次科技动作才按新主人的科技更新，
  期间数值还是对面玩家的。

---

## 6. 升级机制（本模块的核心）

### 6.1 要解决的问题

- 研究「长剑兵」后，场上已有的阔剑兵要变成长剑兵；
- 研究之后**新建**的短剑兵，应该直接出长剑兵（内核传的是底层兵种）；
- 招降来的高级单位不能被降级。

### 6.2 三个要素

| 要素 | 作用 |
|---|---|
| `ObjectCanImprove(sort, type)` | `type` → 槽位号 `code`；没有升级链返回 `-1` |
| `__OBJECTREALLYTYPE__[code]` | 该槽位**目前已经升到**的 `type`；初值是该链的底层类型 |
| `redirectObjectType(sort, type)` | 新建/升级时把 `type` 改判成槽位里的当前级别 |
| `changeObjectType(sort, code)` | 把场上所有属于该槽位的对象换成当前级别 |

`redirectObjectType` 的判定：

```cpp
I32 tmp = ObjectCanImprove(sort, type);
if (tmp < 0 || tmp >= SIZE) return type;      // 没有升级链
if (__OBJECTREALLYTYPE__[tmp] <= type) return type;  // 槽位还没升过（或就是同级）
return __OBJECTREALLYTYPE__[tmp];            // 抬到槽位当前级别
```

`<=` 这个比较依赖**同一枚举内「升级后的值一定大于升级前」**。
`AT_*` 与 `BUILDING_*` 的现有链都满足（见 `Public/var/Army.h`、`Building.h` 的排列），
以后加链时必须遵守，否则判断会失效。

### 6.3 槽位表

编号在 `Player.h` 的 `__OBJECTREALLYTYPE__` 初始化列表里，**兵种/农民 0~13，箭塔 20，城墙 21**：

| code | 链（初值 → 最高） |
|---:|---|
| 0 | 小型渔船 → 大型渔船 |
| 1 | 小型商船 → 大型商船 |
| 2 | 小型运输船 → 重型运输船 |
| 3 | 棍棒兵 → 刀斧兵 |
| 4 | 短剑兵 → 阔剑兵 → 长剑兵 → 军团步兵 |
| 5 | 强弓兵 → 复合弓兵 |
| 6 | 弓骑兵 → 重装弓骑兵 |
| 7 | 骑兵 → 重装骑兵 → 铁甲骑兵 |
| 8 | 战车 → 刀轮战车 |
| 9 | 战象 → 装甲战象 |
| 10 | 投石车 → 中型投石车 → 重型投石车 |
| 11 | 弩炮 → 重型弩炮 |
| 12 | 侦察船 → 桨帆船 → 三列桨帆船 |
| 13 | 方阵兵 → 十夫长 → 百夫长 |
| 20 | 瞭望箭塔 → 警戒箭塔 → 防御箭塔 → 防御炮塔 |
| 21 | 初级城墙 → 中级城墙 → 高级城墙 |

`__OBJECTREALLYTYPE__` 是**每个玩家一份**（非 static），因为升级进度是玩家自己的状态。

城墙和兵种一样，属于「升级后场上已有的同类对象一起换掉」：研究中级城墙后，
已有的初级城墙会直接变成中级城墙（已确认，不是漏了开关）。

### 6.4 时序

```text
notifyBuildingActionFinish(object, action)
│
├─ object 已失效 或 是造人动作 ─────────────▶ return（与科技无关）
│
├─ 科技类动作（升时代 / MARKET / STOCK /
│    GOVERNMENT / TEMPLE）  ───────────────▶ refreshAllObjectScience(); return
│
└─ 升级类动作
     ├─ 查 switch 得到 (Sort, Code, RealType)
     ├─ __OBJECTREALLYTYPE__[Code] = RealType      // 先记下新级别
     └─ changeObjectType(Sort, Code)               // 再把场上的对象换过去
          └─ 遍历对应容器：
               ObjectCanImprove(obj) == Code  →  obj->changeType(redirectObjectType(obj))
```

`changeType` 内部会刷新名称、视野、数值、碰撞盒，所以升级后不需要再单独刷新。

**顺序不能反**：先写槽位，`redirectObjectType` 才能取到新级别。

### 6.5 为什么建筑用 20/21

`__OBJECTREALLYTYPE__` 按 `code` 索引，早期的表让箭塔和学院单位共用 `13`：

- 塔升级 → `[13] = BUILDING_SENTRY_TOWER`（值为 7）→ 方阵兵判断 `7 <= 32` 成立 → **永远升不了级**；
- 学院升级 → `[13] = AT_PHALANX`（33）→ 新建塔判断 `33 <= 6` 不成立 → **塔被改成 33 号（一个步兵）**。

两者分属不同枚举，取值空间本来就不该混用，所以建筑单独占用 20/21，
中间 14~19 留空，方便以后插入新的建筑链。

### 6.6 谁会被升级、谁不会

| 情况 | 行为 |
|---|---|
| 场上属于该槽位的低级对象 | 被 `changeObjectType` 升级 |
| 场内已经更高级的对象（如招降来的） | 不受影响（`ObjectCanImprove` 查不到它，或 `redirectObjectType` 判断为同级） |
| 招降来的对象 | 新建时不经过 `redirectObjectType`，级别保持原样 |
| 投掷物、农田 | 不参与任何升级 |

### 6.7 目前没有接进来的动作

| 动作 | 现状 |
|---|---|
| `BUILDING_RANGE_UPGRADE_IMPROVEDBOW`（升级改良弓手） | 不在 switch 里，等于空操作；槽位 5 的初值是 `AT_IMPROVED_BOWMAN`，由「复合弓」动作负责抬到 `AT_COMPOSITE_BOWMAN` |
| `BUILDING_GRANARY_WALL`（解锁初级城墙）、`BUILDING_GRANARY_ARROWTOWER`（解锁瞭望塔） | 不升级任何对象；「能不能建」由 `Development::buildingCanBuild` 判断，Player 只管新建时的级别（初值就是 1 级） |

---

## 7. 与内核对齐的约定

1. **对象由内核创建/销毁**，Player 只登记。内核要造兵：先扣资源、算出生点，再调 `newArmy(pos, type, science)`。
2. **动作完成后内核要通知 Player**：`notifyBuildingActionFinish(object, action)`。
   `object` 必须是有效对象（空 `WeakPtr` 会直接 return）。
3. **通知顺序**：内核应**先**让 `Development` 更新完科技状态，**再**调 Player。
   反过来的话，`refreshAllObjectScience` 重算到的还是旧值。
   （当前 Player 不再自己转发给 `Development`，见 §8。）
4. **内核保证动作真的完成了才通知**：Player 不维护「进行中的动作」，也不做幂等保护。
   同一个动作被通知两次，对象会被再升一级（如果该槽位还有下一级）。

---

## 8. 已知问题 / 待办

| # | 问题 | 说明 |
|---|---|---|
| 1 | 依赖 `Development::get_speed_attack` | `Human` / `Building` 的 `refreshAttack` 用到它，科技模块合入前 `BaseObject` 编译不过 |
| 2 | `tick()` 为空 | 不再清理空槽位 |
| 3 | `__OBJECTREALLYTYPE__` 用了双下划线前缀 | 双下划线是 C++ 保留标识符；`__BuildingActionIsCreatHuman` 同样问题，且少个 `e`（应为 `Create`） |
| 4 | `removeXXX(I32)` 与 `SN(U64)` 类型不一致 | 比较时 `I32` 被提升为 `U64`，负数会变成极大值；实际不会传负数，但类型上不严谨 |
| 5 | `refreshAllObjectScience` 不刷投掷物与农田 | 有意为之：投掷物发射时定值，农田按建成时科技定格 |
| 7 | `ObjectCanImprove` / `changeObjectType` 命名不统一 | 一个驼峰一个下划线，属风格问题 |

---

## 9. 加一条新升级链要改哪些地方

以「骑兵 → 重装骑兵」为例，共 4 处：

1. **`ObjectCanImprove`**：把下层的 `type` 加到对应 `code` 的 `case` 里
   （`AT_CAVALRY` / `AT_HEAVY_CAVALRY` → `7`）。
2. **`__OBJECTREALLYTYPE__` 初值**：该 `code` 的初值必须是链的**底层类型**（`AT_CAVALRY`）。
   新开一个 `code` 时，还要同步改 `__OBJECTREALLYTYPE_SIZE__`。
3. **`notifyBuildingActionFinish` 的 switch**：加上动作 → `(Sort, Code, RealType)`。
   `RealType` 必须是升级后的 `type`，且在同一枚举里**大于**升级前的值。
4. **如果这个动作还会影响「没有升级的对象」**，要额外处理。
   升级类动作走的是 `changeObjectType`，只对被升级的那些对象调 `changeType()`（内部自带 `refresh()`），
   **不会**调用 `refreshAllObjectScience()`。
   也就是说：被升级的单位会拿到新数值，但同类的其它单位不会重算。
   如果新动作像「建筑学：所有建筑生命 +20%」那样是全局效果，就不能放进入升级分支，
   应归到前面的「科技类动作」`case` 列表里（那条路径以 `refreshAllObjectScience(); return;` 结束）。

---

## 10. 快速自检

```cpp
// 期望：新建骑兵还是骑兵；研究重骑兵科技后，已存在的骑兵变重装骑兵，弓骑兵不受影响
Army* a = p.newArmy(pos, AT_CAVALRY, dev);
cout << a->getType();                                   // AT_CAVALRY
p.notifyBuildingActionFinish(tower, BUILDING_STABLE_UPGRADE_CAVALRY);
cout << a->getType();                                   // AT_HEAVY_CAVALRY
```
