//
// Created by cyx on 2019/8/30.
//
#include "Character.h"

#include <utility>
#include <fstream>

#include "Status.h"
#include "Item.h"
#include "Tool.h"
#include "Scene.h"
#include "templateHeader.h"
#include "GameLoop.h"
#include "global.h"

extern vector<NPC>globalNPC;
extern unique_ptr<Map>mapNow;
extern SCOORD uPos;
extern unique_ptr<Map>mapNow;
// ----------------------Character类----------------------
/*
 * @brief 完整的构造函数
 *
 * @param status:状态
 *        id:标识
 *        nameCN:中文名
 *        nameEN:英文名
 *        fallingExp:掉落经验
 *        description:描述
 *        location:位置
 *        display:地图上的显示
 */
Character::Character(Status status,
                     string id,
                     string nameCN,
                     string nameEN,
                     string description,
                     Location location,
                     char display,
                     int fallingExp,
                     int fallingMoney) {
    this->status = status;
    this->id = id;
    this->nameEN = nameEN;
    this->nameCN = nameCN;
    this->description = description;
    this->mapLocation = location;
    this->displayChar = display;
    this->fallingExp = fallingExp;
    this->fallingMoney = fallingMoney;
}

/*
 * @brief 简易的构造函数
 * 所有的初始化为空
 *
 */
Character::Character():mapLocation({1,1,1}) {
    Status status1;
    this->status = status1;
    this->nameCN = "";
    this->nameEN = "";
    this->fallingMoney = 0;
    this->fallingExp = 0;
    this->description = "无";
    this->displayChar = 'a';
}

/*
 * @brief 判断是否死亡
 *
 * @return 返回bool类型, true表示死亡
 */
bool Character::isDead() {
    return (this->status.HP <= 0);
}


/*
 * @brief 展示信息
 *
 */
void Character::showDescription() {
    cout << this->description << endl;
}

// ---------------------Monster类-----------------------

Monster::Monster(string id) :Character(){
    this->id = id;
    ifstream f(READ_MONSTER_PATH);
    string str;

    while (getline(f, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && idLine[1] == id) {
                break;
            }
        }
    }
    map<string, string> data = Tool::dataMap(f);

    this->nameCN = data["nameCN"];
    this->description = data["description"];
    this->nameEN = data["nameEN"];
    this->fallingMoney = fromString<int>(data["fallingMoney"]);
    this->fallingExp = fromString<int>(data["fallingExp"]);
    this->displayChar = fromString<char>(data["displayChar"]);

    string items = data["fallingItem"];
    if (items != "None"){
        vector<string>temp;
        temp = Tool::split(items, ' '); // 空格隔开
        for (auto iter = temp.begin(); iter != temp.end(); iter++) {
            Item aItem(fromString<int>(*iter));
            this->fallingItem.push_back(aItem);
        }
    }
    f.close();
    status.loadStatus(id,READ_MONSTER_STATUS_PATH);
}


// --------------------Player类-------------------------

/*
 * @brief 构造函数
 */
Player::Player() :Character(){
    this->id = "PY-00"; // 固定的id
    this->experiencePoint = 0;
    this->days = 0;
    this->money = 0;
    this->Lv = 1;
    this->weapon = Weapon();
    this->armor = Armor();
    this->maxMP = 150;
    this->maxMP = 100;
    this->status.HP = 150;
    this->status.MP = 100;
    this->status.ATK = 20;
    this->status.DEF = 20;
    this->status.Speed = 5;
    this->status.Critical = 50;
    this->status.Phy = 5;
}

/*
 * @brief 增加玩家的钱
 *
 * @param addition:增加数额
 */
void Player::addMoney(int addition) {
    if (money < 100000){
        money += addition;
    }
    else{
        cout << "金钱到达上限" << endl;
    }
}

/*
 * @brief 增加经验,顺便检查是否升级了
 *
 * @param addition:增加数额
 */
void Player::addExp(int addition) {
    if(Lv > 20){
        cout << "满级无法增加经验" << endl;
        return;
    }
    experiencePoint += addition;
    levelUp();
    cout << "当前等级" << Lv << endl;
}
/*
 * @brief 升级函数
 * 经验需求按线性增长
 * @return 是否升级
 */
void Player::levelUp() {
    int nextLvExp = 100 * (Lv + 1);
    while (experiencePoint >= nextLvExp){
        experiencePoint -= nextLvExp;
        cout << nameCN << "升级了" << endl;
        Lv += 1;
        if (Lv > 20){
            cout << "达到了最高等级" << endl;
        }
        this->status.DEF += 5;
        this->status.ATK += 5;
        this->maxHP += 15;
        this->maxMP += 15;
        this->status.Speed += 1;
        this->status.HP = maxHP;
        this->status.MP = maxMP;
        if (Lv == 2){
            // 学习新的技能
            cout << "学会了新的技能:攻击态势" << endl;
            Skill newSkill("B02");
            this->skills.push_back(newSkill);
        }
        if(Lv == 3){
            cout << "学会新的技能:一刀999" << endl;
            Skill newSkill("A02");
            this->skills.push_back(newSkill);
        }
        nextLvExp = 100 * (Lv + 1);
    }
    cout << "剩余 " << nextLvExp - experiencePoint << " 经验升到下一级" << endl;
}

/*
 * @brief 脱下装备
 */
bool Player::takeOffArmor(string &name) {
    if (this->armor.id == 0){
        cout << "当前无装备" << endl;
        return false;
    }
    else{
        if (this->armor.nameEN != name){
            cout << "未装备指定的防具" << endl;
            return false;
        }
        if(!armorBag.addItem(this->armor.id, 1)){
            cout << "背包满了" << endl;
            return false;
        }
        else{
            cout << "成功脱下装备" << this->armor.nameCN << endl;
            this->status.ATK -= armor.ATK;
            this->status.DEF -= armor.DEF;
            this->status.Speed -= armor.Speed;
            this->armor = Armor();
            return true;
        }
    }
}

bool Player::takeOffWeapon(string &name) {
    if (this->weapon.id == 0){
        cout << "当前无装备" << endl;
        return false;
    }
    else{
        if (this->weapon.nameEN != name){
            cout << "未装备指定的防具" << endl;
            return false;
        }
        if(!weaponBag.addItem(this->weapon.id, 1)){
            cout << "背包满了" << endl;
            return false;
        }
        else{
            cout << "成功脱下装备" << this->weapon.nameCN << endl;
            // 空装备
            this->status.ATK -= weapon.ATK;
            this->status.DEF -= weapon.DEF;
            this->status.Speed -= weapon.Speed;
            this->weapon = Weapon();
            return true;
        }
    }
}

/*
 * @brief 装备防具
 * 卸下防具就是装备空防具
 * @param 命令行传入的字符串name
 */
bool Player::equipArmor(string &name) {
    Armor oldArmor = this->armor;
    for (auto iter = armorBag.items.begin(); iter != armorBag.items.end() ; iter++) {
        if ((*iter).nameEN == name){
            this->armor = *iter;
            // 处理加成
            this->status.ATK += ((*iter).ATK - oldArmor.ATK);
            this->status.DEF += ((*iter).DEF - oldArmor.DEF);
            this->status.Speed += ((*iter).Speed - oldArmor.Speed);
            // 当前有装备
            if (oldArmor.id != 0){
                cout << "卸下了" << oldArmor.nameCN << endl;
                armorBag.addItem(oldArmor.id, 1);
            }
            cout << "防具" << (*iter).nameCN << "装备成功" << endl;
            armorBag.deleteItem((*iter).id, 1);
            return true;
        }
    }
    cout << "无该装备" << endl;
    return false;
}

/*
 * @brief 装备武器
 * @param 命令行传入的字符串name
 */
bool Player::equipWeapon(string& name) {
    Weapon oldWeapon = this->weapon;
    for (auto iter = weaponBag.items.begin(); iter != weaponBag.items.end() ; iter++) {
        if ((*iter).nameEN == name){
            this->weapon = *iter;
            // 处理加成
            this->status.ATK += ((*iter).ATK - oldWeapon.ATK);
            this->status.DEF += ((*iter).DEF - oldWeapon.DEF);
            this->status.Speed += ((*iter).Speed - oldWeapon.Speed);
            if (oldWeapon.id != 0){
                cout << "卸下了" << oldWeapon.nameCN << endl;
                weaponBag.addItem(oldWeapon.id, 1);
            }
            cout << "武器" << (*iter).nameCN << "装备成功" << endl;
            weaponBag.deleteItem((*iter).id, 1);
            return true;
        }
    }
    cout << "无该装备" << endl;
    return false;
}

/*
 * @brief 给定物品展示物品的详细信息
 */
bool Player::showItem(int itemId) {
    if (itemId < 100){
        // 武器
        for (auto iter = weaponBag.items.begin(); iter != weaponBag.items.end() ; iter++) {
            if (iter->id == itemId){
                iter->showDescription();
                return true;
            }
        }
        return false;
    }
    if (itemId < 200){
        // 防具
        for (auto iter = armorBag.items.begin(); iter != armorBag.items.end(); iter++) {
            if (iter->id == itemId){
                iter->showDescription();
                return true;
            }
        }
        return false;
    }
    if (itemId < 300){
        // 药品
        for (auto iter = drugBag.items.begin(); iter != drugBag.items.end(); iter++) {
            if (iter->id == itemId){
                iter->showDescription();
                return true;
            }
        }
        return false;
    }
    if (itemId < 400){
        // 物品
        for (auto iter = itemBag.items.begin(); iter != itemBag.items.end() ; iter++) {
            if (iter->id == itemId){
                iter->showDescription();
                return true;
            }
        }
        return false;
    }
    return false;
}

/*
 * @brief 展示防具背包
 */
void Player::showArmors() {
    cout << "玩家的防具背包" << endl;
    armorBag.showItems();
}

/*
 * @brief 展示武器背包
 */
void Player::showWeapons() {
    cout << "玩家的武器背包" << endl;
    weaponBag.showItems();
}
/*
 * @brief 展示药物背包
 */
void Player::showDrugs() {
    cout << "玩家的药物" << endl;
    drugBag.showItems();
}

/*
 * @brief 展示物品背包
 */
void Player::showItems() {
    cout << "玩家的道具" << endl;
    itemBag.showItems();
}
/*
 * @brief 接受任务,同时检查是否已经接受过该任务
 *
 * @param mission:任务的引用
 * @return 是否接受成功
 */
bool Player::addMission(Mission &mission) {
    for (auto iter = quests.begin(); iter != quests.end(); iter++) {
        if ((*iter).id == mission.id){
            cout << "任务进行中" << endl;
            return false;
        }
    }
    quests.push_back(mission);
    cout << "接受任务成功" << endl;
    return true;
}
/*
 * @brief 提示任务信息
 */
void Player::showMissions() {
    if(quests.empty()){
        cout << "没有接任何任务" << endl;
        return;
    }
    for (auto iter = quests.begin(); iter != quests.end(); iter++) {
        cout << "任务" << (*iter).id << (*iter).nameCN << "(" << (*iter).nameEN << ")" << endl;
        cout << "进度:";
        if ((*iter).isProcess){
            cout << "进行中" << endl;
        }
        if((*iter).isFinished){
            cout << "完成" << endl;
        }
    }
}
/*
 * @brief 展示当前进行中的任务
 *
 * @param missionId: 任务的id
 */
void Player::showMission() {
    cout << "-------当前进行中的任务:-------" << endl;
    bool flag = false;
    for (auto iter = quests.begin(); iter != quests.end(); iter++) {
        if ((*iter).isProcess){
            flag = true;
            iter->showDescription();
        }
    }
    if (!flag){
        cout << "没有进行中的任务!" << endl;
    }

}
/*
 * @brief 获取任务
 * 获取的是委托人的第一个未完成的任务
 * @param assigner: 委托人的名字的英文
 * @return 任务的指针, 找不到就返回空指针
 */
Mission* Player::getMission(string &assignerName) {
    if (quests.empty()){
        return nullptr;
    }
    for (auto iter = quests.begin(); iter != quests.end() ; iter++) {
        if ((*iter).assigner == assignerName && !((*iter).isFinished)){
            return &(*iter);
        }
    }
    return nullptr;
}
/*
 * @brief 获取任务对于传参的重载
 *
 * @param 任务的id
 * @return 返回指定任务的指针,不存在返回空指针
 */
Mission* Player::getMission(int missionId) {
    if (quests.empty()){
        return nullptr;
    }
    for (auto iter = quests.begin(); iter != quests.end() ; iter++) {
        if ((*iter).id == missionId){
            return &(*iter);
        }
    }
    return nullptr;
}

/*
 * @brief 展示玩家的信息
 */
void Player::showStatus() {
    cout.setf(ios::left);
    cout <<  setw(10) <<"玩家姓名:" << this->nameCN << endl;
    cout <<  setw(7) << "等级:" << this->Lv << endl;
    cout <<  setw(7) << "生命:" << this->status.HP << endl;
    cout <<  setw(7) << "法力值:" << this->status.MP;
    cout <<  setw(7) << "速度:" << this->status.Speed;
    cout <<  setw(7) << "防御:" << this->status.DEF;
    cout <<  setw(7) << "攻击:" << this->status.ATK << endl;
    cout.unsetf(ios::left);
}

/*
 * @brief 个人信息面板
 */
void Player::playerMenu() {
    cout << "玩家菜单" << endl;
    cout << "你可以使用help命令查看可使用命令" << endl;
}

/*
 * @brief 添加物品
 * 根据id分类添加
 * @param itemId:物品的id number:要添加的个数
 */
void Player::addItem(int itemId, int number) {
    int division = itemId / 100;
    if (division == 0){
        weaponBag.addItem(itemId, number);
    }
    else if (division == 1){
        armorBag.addItem(itemId, number);
    }
    else if (division == 2){
        drugBag.addItem(itemId, number);
    }
    else if (division == 3){
        itemBag.addItem(itemId, number);
    }
    else{
        cout << "错误的id" << endl;
    }
}

/*
 * @brief 根据id分类删除
 *
 * @param itemId:物品id number:要删除的个数
 */
void Player::deleteItem(int itemId, int number) {
    int division = itemId / 100;
    if (division == 0){
        weaponBag.deleteItem(itemId, number);
    }
    else if (division == 1){
        armorBag.deleteItem(itemId, number);
    }
    else if (division == 2){
        drugBag.deleteItem(itemId, number);
    }
    else if (division == 3){
        itemBag.deleteItem(itemId, number);
    }
    else{
        cout << "错误的id" << endl;
    }
}

/*
 * @brief 完全删除某个物品
 * 用于命令行的丢弃命令
 * @param itemId: 物品的id
 */
void Player::eraseItem(int itemId) {
    int division = itemId / 100;
    if (division == 0){
        weaponBag.deleteItem(itemId);
    }
    else if (division == 1){
        armorBag.deleteItem(itemId);
    }
    else if (division == 2){
        drugBag.deleteItem(itemId);
    }
    else if (division == 3){
        itemBag.deleteItem(itemId);
    }
    else{
        cout << "错误的id" << endl;
    }
}

/*
 * @brief 用于获取指定id物品的个数
 *
 * @param itemId: 物品的id
 * @return 物品的个数,0表示没有
 */
int Player::getItem(int itemId) {
    int division = itemId / 100;
    if (division == 0){
        for (unsigned int i = 0; i < weaponBag.items.size(); i++) {
            if (weaponBag.items[i].id == itemId){
                return weaponBag.items[i].num;
            }
        }
    }
    else if (division == 1){
        for (unsigned int i = 0; i < armorBag.items.size(); i++) {
            if (armorBag.items[i].id == itemId){
                return armorBag.items[i].num;
            }
        }
    }
    else if (division == 2){
        for (unsigned int i = 0; i < drugBag.items.size(); i++) {
            if (drugBag.items[i].id == itemId){
                return drugBag.items[i].num;
            }
        }
    }
    else if (division == 3){
        for (unsigned int i = 0; i < itemBag.items.size(); i++) {
            if (itemBag.items[i].id == itemId){
                return itemBag.items[i].num;
            }
        }
    }
    else{
        cout << "错误的id" << endl;
    }
    return 0;
}

/*
 * @brief 判断是否死亡
 * 可以考虑扩展一些东西
 * @return 是否死亡
 */
bool Player::isDead() {
    if (this->status.HP <= 0){
        string input;
        cout << "你处于濒死状态" << endl;
        cout << "要复活吗(y/n):";
        cin >> input;
        if (input == "y"){
            this->status.HP += 100;
            // 清除缓存区
            cin.clear();
            cin.ignore();
            return false;
        }
        else{
            // 清除缓存区
            cin.clear();
            cin.ignore();
            return true;
        }
    }
    return false;
}

/*
 * @brief 玩家的存档函数
 */
void Player::save() {
    ofstream of;
    of.open(SAVE_PLAYER_PATH);
    map<string,string> m_map;

    //保存player的单项属性
    //m_map["type"] = "attribute";
    of << "type attribute" << endl;
    m_map["id"] = id;
    m_map["nameCN"] = nameCN;
    m_map["nameEN"] = nameEN;
    m_map["description"] = description;
    m_map["fallingExp"] = toString<int>(fallingExp);
    m_map["fallingMoney"] = toString<int>(fallingMoney);
    m_map["displayChar"] = toString<char>(displayChar);
    m_map["experiencePoint"] = toString<int>(experiencePoint);
    m_map["days"] = toString<int>(days);
    m_map["money"] = toString<int>(money);
    m_map["Lv"] = toString<int>(Lv);
    m_map["maxHP"] = toString<int>(maxHP);
    m_map["maxMP"] = toString<int>(maxMP);
    m_map["weapon"] = toString<int>(weapon.id);
    m_map["armor"] = toString<int>(armor.id);
    for(auto iter = m_map.begin(); iter != m_map.end(); iter ++){
        of << iter->first << " " << iter->second << endl;
    }
    m_map.clear();
    of << endl;

    //保存player的location
    //m_map["type"] = "location";
    mapLocation.mapId = mapNow->id;
    mapLocation.x = uPos.X;
    mapLocation.y = uPos.Y;
    of << "type location" << endl;
    m_map["mapId"] = toString<int>(mapLocation.mapId);
    m_map["x"] = toString<int>(mapLocation.x);
    m_map["y"] = toString<int>(mapLocation.y);
    for(auto iter = m_map.begin(); iter != m_map.end(); iter ++){
        of << iter->first << " " << iter->second << endl;
    }
    m_map.clear();
    of << endl;

    //保存player的Status
    status.saveStatus("player");

    //保存player的skills
    of << "type" << " " << "skill" << endl;
    for (auto & skill : skills) {
        of << skill.id << endl;
    }
    of << endl;

    //保存player的weapon
    of << "type" << " " << "weapon" << endl;
    for (auto & item : weaponBag.items) {
        of << toString(item.id) << " " << toString(item.num) << endl;
    }
    of << endl;

    //保存player的armor
    of << "type" << " " << "armor" << endl;
    for (auto & item : armorBag.items) {
        of << toString(item.id) << " " << toString(item.num) << endl;
    }
    of << endl;

    //保存player的drug
    of << "type" << " " << "drug" << endl;
    for (auto & item : drugBag.items) {
        of << toString(item.id) << " " << toString(item.num) << endl;
    }
    of << endl;

    //保存player的item
    of << "type" << " " << "item" << endl;
    for (auto & item : itemBag.items) {
        of << toString(item.id) << " " << toString(item.num) << endl;
    }
    of << endl;

    of.close();

    if(!quests.empty()){
        for (auto & quest : quests) {
            quest.saveMission("player",SAVE_MISSION_PATH);
        }
    } else{
        ofstream of;
        of.open(SAVE_MISSION_PATH);
        of.close();
    }

}

/*
 * @brief buff值累计入玩家的属性
 *
 * @param buff:的引用
 */
void Player::addBuff(Buff &buff) {
    if (buffs.empty()){
        buffs.push_back(buff);
    }
    else{
        // 存在buff,先判断是否重叠
        for (auto iter = buffs.begin(); iter != buffs.end() ; iter++) {
            if ((*iter).name == buff.name){
                (*iter).duration += buff.duration - 1;
                return;
            }
        }
        // 不重叠
        buffs.push_back(buff);
    }
    this->status.Critical += buff.Critical;
    this->status.Speed += buff.Speed;
    this->status.ATK += buff.ATK;
    this->status.DEF += buff.DEF;
    this->status.HP += buff.HP;
}

/*
 * @brief buff值减少玩家的属性
 *
 * @param buff:的引用
 */
void Player::deleteBuff(Buff &buff) {
    this->status.Critical -= buff.Critical;
    this->status.Speed -= buff.Speed;
    this->status.ATK -= buff.ATK;
    this->status.DEF -= buff.DEF;
    this->status.HP -= buff.HP;
}

/*
 * @brief 用于命令行使用
 *
 * @param 传入一个药物的名字
 */
bool Player::useDrug(string& name, Character &character) {
    for (auto iter = drugBag.items.begin(); iter != drugBag.items.end(); iter++) {
        if ((*iter).nameEN == name){
            this->drugBag.deleteItem((*iter).id, 1);
            if((*iter).playerTarget){
                this->status.HP += (*iter).HP;
                if (status.HP > maxHP){
                    status.HP = maxHP;
                }
                cout << "恢复到了" << status.HP << "血" << endl;
                this->status.MP += (*iter).MP;
                if (status.MP > maxMP){
                    status.MP = maxMP;
                }
                cout << "恢复到了" << status.MP << "蓝" << endl;
            }
            else{
                character.status.HP -= iter->HP;
                cout << "造成" << iter->HP << "点伤害" << endl;
            }
            return true;
        }
    }
    return false;
}

/*
 * @brief 玩家死亡的场景
 */
void Player::deadScene() {
    cout << "你死了" << endl;
    cout << "菜" << endl;
}

/*
 * @brief 玩家的载入函数
 */
void Player::load() {
    ifstream os(SAVE_PLAYER_PATH);

    string str;

    //获取单项属性值
    // 找到对应 type 为 attribute 处
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "attribute") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的内容读取为键值对
    map<string, string> data = Tool::dataMap(os);

    this->id = id;
    this->nameCN = data["nameCN"];
    this->nameEN = data["nameEN"];
    this->description = data["description"];
    this->fallingExp = fromString<int>(data["fallingExp"]);
    this->fallingMoney = fromString<int>(data["fallingMoney"]);
    this->displayChar = fromString<char>(data["displayChar"]);
    this->experiencePoint = fromString<int>(data["experiencePoint"]);
    this->days = fromString<int>(data["days"]);
    this->money = fromString<int>(data["money"]);
    this->Lv = fromString<int>(data["Lv"]);
    this->maxMP = fromString<int>(data["maxMP"]);
    this->maxHP = fromString<int>(data["maxHP"]);
    weapon = Weapon(fromString<int>(data["weapon"]),1);
    armor = Armor(fromString<int>(data["armor"]),1);
    data.clear();

    //读取位置属性值
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "location") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的内容读取为键值对
    data = Tool::dataMap(os);

    this->mapLocation.y = fromString<int>(data["y"]);
    this->mapLocation.x = fromString<int>(data["x"]);
    this->mapLocation.mapId = fromString<int>(data["mapId"]);
    data.clear();

    //读取skill
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "skill") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的skills按照id赋值
    while (getline(os, str)) {
        // 读到空行结束
        if (!str.empty()) {
            skills.emplace_back(str);
        } else {
            break;
        }
    }

    //读取weapon
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "weapon") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的weapon按照id赋值
    data = Tool::dataMap(os);
    map<string,string>::iterator iter;
    for (iter = data.begin(); iter != data.end(); iter ++) {
        weaponBag.addItem(fromString<int>(iter->first),fromString<int>(iter->second));
    }
    data.clear();

    //读取armor
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "armor") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的armor按照id赋值
    data = Tool::dataMap(os);
    for (iter = data.begin(); iter != data.end(); iter ++) {
        armorBag.addItem(fromString<int>(iter->first),fromString<int>(iter->second));
    }
    data.clear();

    //读取drug
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "drug") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的drug按照id赋值
    data = Tool::dataMap(os);
    for (iter = data.begin(); iter != data.end(); iter ++) {
        drugBag.addItem(fromString<int>(iter->first),fromString<int>(iter->second));
    }
    data.clear();

    //读取item
    while (getline(os, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "type" && idLine[1] == "item") {
                break;
            }
        }
    }
    // 将 对应 type 行到下一个空行之间的item按照id赋值
    data = Tool::dataMap(os);
    for (iter = data.begin(); iter != data.end(); iter ++) {
        itemBag.addItem(fromString<int>(iter->first),fromString<int>(iter->second));
    }
    data.clear();

    os.close();

    status.loadStatus("player",SAVE_STATUS_PATH);

    ifstream ifstream1;
    ifstream1.open(SAVE_MISSION_PATH);

    int i = 0;
    int flag = 0;
    while (!ifstream1.eof()){
        // 找到对应 owner 处
        while (getline(ifstream1, str)) {
            if (!str.empty()) {
                vector<string> idLine = Tool::split(str);
                if (idLine[0] == "owner" && idLine[1] == "player") {
                    flag = 1;
                    break;
                }
            }
        }
        if (flag == 1){
            data = Tool::dataMap(ifstream1);

            //Mission mission(fromString<int>(data["id"]));
            quests.push_back(Mission(fromString<int>(data["id"])));
            quests[i].isAccepted = fromString<bool>(data["isAccepted"]);
            quests[i].isFinished = fromString<bool>(data["isFinished"]);
            quests[i].isProcess = fromString<bool>(data["isProcess"]);
            i += 1;
        }
        else break;
    }
    ifstream1.close();
}

/*
 * @brief 战斗时背包的显示
 */
void Player::battleBagShow(SCOORD& pos) {
    if (!this->drugBag.items.empty()){
        for (auto iter = drugBag.items.begin(); iter != drugBag.items.end() ; iter++) {
            Map::gotoxy(pos);
            cout << iter->nameCN << "\t" << iter->nameEN << "\t" << " X " << iter->num;
            pos.Y++;
        }
    }
    else{
        Map::gotoxy(pos);
        cout << "\t\t无物品" << endl;
        pos.Y++;
    }

}

/*
 * @brief 展示玩家的技能的信息
 */
void Player::showSkills() {
    for (auto iter = skills.begin(); iter != skills.end() ; iter++) {
        cout << iter->nameCN << "(" << iter->nameEN << ")" << iter->description << endl;
    }
}
/*
 * @brief 获取玩家的防具
 *
 * @return 返回玩家的防具的指针
 * 无防具的时候返回的armor的id为0, 表示空的防具
 */
Armor* Player::getArmor() {
    return &(this->armor);
}

/*
 * @brief 获取玩家的武器
 *
 * @return 武器的指针,当武器的id为0时表示为空武器
 */
Weapon* Player::getWeapon() {
    return &(this->weapon);
}

// -----------------------NPC类-----------------------

/*
 * @brief 保存npc, 会自动判断是否需要保存的
 * 存储任务交给玩家的存储
 * 商店由于静态单独保存
 */
void NPC::save() {
    ofstream fp;
    fp.open(SAVE_NPC_PATH);
    for (auto j = globalNPC.begin(); j != globalNPC.end() ; j++) {
        if (j->needSave){
            fp << "id" << " " << j->id << endl;
            fp << "nameCN" << " " << j->nameCN << endl;
            fp << "shopStatus" << " " << j->shopStatus << endl;
            fp << "missionStatus" << " " << j->missionStatus << endl;
            fp << "bar" << " " << j->bar << endl;
            fp << "isVisible" << " " << j->isVisible << endl;
            fp << "quests" << " ";
            for (unsigned int i = 0; i < (*j).questList.size(); i++) {
                fp << (*j).questList[i].id;
                if (i == (*j).questList.size()-1){
                    continue;
                }
                fp << ",";
            }
            fp << endl;
            fp << endl;
        }
    }
    fp.close();
}

/*
 * @brief 载入npc的动态的属性
 * 必须要先构造npc
 * 文件里存在id的npc
 * 有些npc不需要一些基本属性,就按照默认的设定来
 * <!重要>只有需要读取的时候再调用
 * 商店由于静态单独加载
 */
void NPC::load(string path) {
    // 需要读取
    ifstream fp;
    fp.open(path);
    string str;
    while (getline(fp, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && idLine[1] == this->id) {
                break;
            }
        }
    }
    map<string, string> data = Tool::dataMap(fp);

    this->shopStatus = fromString<bool>(data["shopStatus"]);
    this->battleStatus = fromString<bool>(data["battleStatus"]);
    this->missionStatus = fromString<bool>(data["missionStatus"]);
    this->bar = fromString<bool>(data["bar"]);
    this->isVisible = fromString<bool>(data["isVisible"]);
    vector<string>missions = Tool::split(data["quests"], ',');
    // 读取未完成的任务
    for (auto iter = missions.begin(); iter != missions.end(); iter++) {
        Mission remainMission(fromString<int>(*iter));
        this->questList.push_back(remainMission);
    }
    fp.close();
    // 读入属性
    this->status.loadStatus(this->id, READ_MONSTER_STATUS_PATH);

}

/*
 * @brief 保存一下商店
 * 静态方法
 */
void NPC::storeSave() {
    NPC::store.save();
}
/*
 * @brief 初始化商店
 * 静态方法
 */
void NPC::storeClear() {
    // 清空背包
    NPC::store.drugPackage.items.clear();
    NPC::store.armorPackage.items.clear();
    NPC::store.weaponPackage.items.clear();
}

/*
 * @brief 读取商店
 * @param 指定的文件路径,用于区分新游戏和读取游戏
 */
void NPC::storeLoad(string path) {
    NPC::store.load(path);
}

/*
 * @brief 重载流来读取npc的静态信息
 * 关于文件内的key需要注意:
 * -必须严格按照现在的格式,无法自由修改key
 * end结尾
 * 关于文件内的talk需要注意:
 * -满足格式 <id>:<type>:<sentence>
 * -当id为0时说明为日常对话,此时type参数暂时无用
 * -当id不为0时表示任务的id,此时type对应结构体TalkContent的3个字符串
 * -<!重要>id的顺序一定要0在第一个,相同id的不分顺序,但一定要在一起
 */
istream& operator>>(istream &fpStream, NPC &npc) {
    string temp;
    string line;
    vector<string>t;
    TalkContent talkContent = {"", "", ""};
    int missionId;
    fpStream >> temp >> npc.nameCN;
    fpStream >> temp >> npc.nameEN;
    fpStream >> temp >> npc.description;
    fpStream >> temp >> npc.mapLocation.x;
    fpStream >> temp >> npc.mapLocation.y;
    fpStream >> temp >> npc.mapLocation.mapId;
    fpStream >> temp >> npc.displayChar;
    fpStream >> temp >> npc.fallingExp;
    fpStream >> temp >> npc.fallingMoney;
    fpStream >> temp >> npc.needSave;
    NPC::readLastLine += 10;
    int lastId = 0;
    fpStream >> temp;
    while (temp != "end"){
        fpStream >> line;
        // 读入信息说明读完了这一行
        NPC::readLastLine++;
        t = Tool::split(line, ':');
        missionId = fromString<int>(t[0]);
        // 读取的id不同, 说明可以存一波了
        if (lastId != missionId){
            npc.talkContent.insert(make_pair(lastId, talkContent));
            // 清空
            talkContent = {"", "", ""};
        }

        if (missionId != 0){
            if (t[1] == "start"){
                talkContent.start = t[2];
            }
            if(t[1] == "end"){
                talkContent.end = t[2];
            }
            if (t[1] == "process"){
                talkContent.process = t[2];
            }
        }
        else{
            // 平时对话就直接调用start
            talkContent.start = t[2];

        }
        lastId = missionId;
        fpStream >> temp;
        if (temp == "end"){
            // 后面没有了
            npc.talkContent.insert(make_pair(missionId, talkContent));
            // end占一行
            NPC::readLastLine++;
        }
    }
    return fpStream;
}

/*
 * @brief NPC的构造函数
 * 初始化一个空的Character类,后续值由NPC构造函数填充
 * @param id:NPC的id, path:读取文件的路径,用于区分新游戏还是读取NPC
 */
NPC::NPC(string id, string path):Character() {
    this->id = id;
    this->needSave = false;
    ifstream fp;
    string line;
    int round = 0;
    fp.open(NPC_FILE_PATH);
    while (round < NPC::readLastLine){
        getline(fp, line);
        round++;
    }
    while (fp.peek() != EOF){
        getline(fp, line);
        readLastLine++;
        if (line == "------"){
            getline(fp, line);
            readLastLine++;
            vector<string>t = Tool::split(line, ' ');
            if (t[0] == "id" && t[1] == id){
                fp >> *this;
                fp.close();
                if (this->needSave){
                    this->load(path);
                }
                else{
                    // 商店npc
                    if (id == "NN-07"){
                        this->shopStatus = true;
                    }
                    else{
                        this->shopStatus = false;
                    }
                    // 设计爆炸, 打个垃圾补丁
                    if (id == "NY-01"){
                        this->status.loadStatus(this->id, READ_MONSTER_STATUS_PATH);
                    }
                    this->missionStatus = false;
                    this->battleStatus = false;
                    this->bar = false;
                    this->isVisible = true;
                }
                return;
            }
            else{
                continue;
            }
        }
        else{
            continue;
        }
    }
    // 应该替换为try-catch更好?
    fp.close();
    cout << NPC::readLastLine;
    cout << this->id;
    system("pause");
    exit(1);
}

/*
 * @brief 用户菜单
 *
 * @return 返回布尔表示是否跳过命名行的输入
 */
bool NPC::NPCMenu(Player &player) {
    // 特殊场景检查
    if (this->specialEvent(player)){
        return true;
    }
    this->showDescription();
    // 提示用户
    cout << "你可以使用help命令查看当前可交互指令" << endl;
    // 接任务的面板
    if (this->missionStatus){
        if (player.getMission(this->nameEN) != nullptr){
            cout << "当前任务进行中" << endl;
            return false;
        }
        else{
            if(!questList.empty()){
                cout << "有可以接受的任务:" << endl;
                cout << questList[0].nameCN << endl;
            }
        }
        return false;
    }
    // 商店有单独的商店界面
    return false;
}

/*
 * @brief 商店的主页面
 */
void NPC::shopMenu(Player &player) {
    // 对话要简单
    SCOORD curPos = {40, 1};
    for (int i = 0; i < 90; i++) {
        cout << "*";
    }
    curPos.X = 40;
    for (int j = 1; j <= 27; j++) {
        Map::gotoxy(curPos);
        cout << "*";
        curPos.Y++;
    }
    curPos = {0, 1};
    for (int j = 1; j < 10; j++) {
        Map::gotoxy(curPos);
        cout << "*";
        curPos.Y++;
    }
    Map::gotoxy(curPos);
    for (int k = 0; k < 40; k++) {
        cout << "*";
    }

    curPos = {18,1};
    Map::gotoxy(curPos);
    cout << this->nameCN;

    curPos = {10,6};
    Map::gotoxy(curPos);
    cout << "你拥有的钱: " << player.money << endl;

    curPos = {41, 1};
    Map::gotoxy(curPos);
    store.shopMenu(curPos);

    curPos = {0, 11};
    Map::gotoxy(curPos);
}

/*
 * @brief 获取NPC的商店状态
 *
 * @return 返回NPC是否可以开出商店
 * true表示可以进行交易
 */
bool NPC::getShopStatus() {
    return this->shopStatus;
}

/*
 * @brief 和npc交易的函数,需要提前检查是否可以交易
 *
 * @param itemId:物品id number:购买个数 Player:玩家的引用
 */
bool NPC::buy(int itemId, int number, Player &player) {
    if (!this->shopStatus){
        cout << "无法交易" << endl;
        return false;
    }
    if (store.buy(itemId, number, player.money)){
        cout << "买入成功" << endl;
        player.addItem(itemId, number);
        return true;
    }
    else{
        cout << "买入失败" << endl;
        return false;
    }
}

/*
 * @brief 卖出物品的函数
 *
 * @param itemId:物品id number:购买个数 Player:玩家的引用
 */
bool NPC::sell(Item &item, int number, Player &player) {
    if (!this->shopStatus){
        cout << "无法交易" << endl;
        return false;
    }
    store.sell(item, number, player.money);
    return true;
}

/*
 * @brief 判断是否死亡
 * 如果死亡置为不可见
 */
bool NPC::isDead() {
    if (this->status.HP <= 0){
        this->isVisible = false;
        return true;
    }
    return false;
}

/*
 * @brief 多态的展示描述
 */
void NPC::showDescription() {
    cout << "NPC:" << this->nameCN << "(" << this->nameEN << ")" << endl;
    cout << "简介:" << this->description << endl;
}

/*
 * @brief 接受任务, 检查npc任务列表的第一个是否被接受
 *
 * @param player 玩家的引用
 */
void NPC::assignQuest(Player &player) {
    if (!missionStatus){
        cout << "无法接受任务" << endl;
        return;
    }
    if(!questList.empty()){
        if (player.getMission(this->nameEN)){
            cout << "当前委托人进行的任务未完成" << endl;
            return;
        }
        else {
            // 插入一个场景
            if (questList[0].id == 6){
                Scene s(4);
                s.displayScene();
            }
            // 正常任务
            questList[0].isAccepted = true;
            questList[0].isProcess = true;
            // 接任务时的对话
            cout << talkContent[questList[0].id].start << endl;
            player.addMission(questList[0]);
            // 删除这个任务
            questList.erase(questList.begin());

            return;
        }
    }
    cout << "没有任务可以接了" << endl;
}

/*
 * @brief 提交任务
 *
 * @param player:玩家的引用
 */
void NPC::finishQuest(Player &player) {
    if (!missionStatus){
        cout << "无法提交任务" << endl;
    }
    // 获取任务
    Mission* acceptedMission = player.getMission(this->nameEN);
    // 判断是否获取成功
    if (acceptedMission == nullptr){
        cout << "没有可以提交的任务" << endl;
    }
    else{
        // 检查是否可以提交
        // 特殊提交不能在当前npc处提交
        int t = acceptedMission->id;
        if (t == 2|| t == 4 || t == 5 || t == 7 || t == 8 || t == 10 || t == 11){
            cout << "没有任务可以提交" << endl;
            return;
        }

        // 若可以则要修改任务属性为完成
        if (acceptedMission->checkFinished()){
            cout << "任务" << acceptedMission->nameCN << "完成" << endl;
            // 任务结算
            // 任务6的场景
            if (acceptedMission->id == 6){
                // 有日记
                if (player.getItem(308)){
                    Scene s(5);
                    s.displayScene();
                }
                else{
                    Scene s(8);
                    s.displayScene();
                }
            }
            cout << this->talkContent[acceptedMission->id].end << endl;
            acceptedMission->isFinished = true;
            acceptedMission->isProcess = false;
            // 玩家 增加金钱 和 经验
            player.addMoney(acceptedMission->bonusMoney);
            player.addExp(acceptedMission->bonusExperiencePoint);
            if (!questList.empty()){
                cout << "有新的任务可接" << endl;
            }
        }
    }
}

/*
 * @brief 设置npc是否在地图上隐藏
 */
void NPC::setVisibility(bool isVisible) {
    this->isVisible = isVisible;
}

/*
 * @brief 获取npc是否隐藏
 * 地图函数需要调用
 * @return true表示可以显示
 */
bool NPC::getVisibility() {
    return this->isVisible;
}

/*
 * @brief 和NPC的对话
 * 不同任务的进行阶段的对话不同
 */
void NPC::talk(Player &player) {
    // 可以发任务的才有不同的对话
    if (missionStatus){
        Mission *mission = player.getMission(this->nameEN);
        if (mission != nullptr){
            int id = mission->id;
            if(mission->isProcess){
                // 进行中时的对话
                cout << talkContent[id].process << endl;
                return;
            }
        }
    }
    // 平时谈话, 注意平时谈话是start
    cout << talkContent[0].start << endl;
}

/*
 * @brief 用于特殊检查
 * 设计的遗留问题在这里完全崭新
 * @param player:玩家的引用
 * @return 是否跳过界面的对话
 */
bool NPC::specialEvent(Player &player) {
    Mission *mission;
    if (this->id == "NN-10"){
        if (player.getItem(302)){
            cout << "王国的走狗, 我和你没什么好说的" << endl;
            system("pause");
            system("cls");
            return true;
        }
        return false;
    }

    if (this->id == "NN-04" && !player.getMission(1) && player.getItem(302)){
        // 第二条主线的开启
        // 开启条件是没有接受任务1
        Scene scene(3);
        scene.displayScene();
        return true;
    }
    // 接了第二个任务
    if (this->id == "NY-02"){
        if (player.getItem(302)){
            // 有通行证直接开打
            GameLoop::battleLoop(globalNPC[16]);
            mission = player.getMission(2);
            mission->missionFinish(player);
            system("pause");
            system("cls");
            // 从地图上删除
            SCOORD npcPos = {short(globalNPC[16].mapLocation.x), short(globalNPC[16].mapLocation.y)};
            string type = "npc";
            mapNow->deleteBarrier(npcPos, type);
            return true;
        }
        else{
            mission = player.getMission(7);
            if (mission!= nullptr && !mission->isFinished){
                cout << this->talkContent[7].end << endl;
                mission->missionFinish(player);
                if (!this->questList.empty()){
                    cout << "有新的任务可接" << endl;
                }
                system("pause");
                system("cls");
                return true;
            }
        }
    }
    // 和梅林对话
    if (this->id == "NN-08"){
        mission = player.getMission(9);
        if (mission != nullptr && !mission->isFinished){
            // 和芙芙的强制战斗
            Monster m("MY-05");
            GameLoop::battleLoop(m);
            cout << this->talkContent[9].end << endl;
            mission->missionFinish(player);
            cout << "有新的任务可接" << endl;
            system("pause");
            system("cls");
            return true;
        }
        mission = player.getMission(8);
        if(mission!= nullptr && !mission->isFinished){
            Scene s(9);
            s.displayScene();
            mission->missionFinish(player);
            cout << "有新的任务可接" << endl;
            system("pause");
            system("cls");
            return true;
        }
    }


    // 和神秘人对话
    mission = player.getMission(5);
    if (this->id == "NN-03" && mission != nullptr &&!mission->isFinished){
        cout << this->talkContent[5].end << endl;
        mission->missionFinish(player);
        // 神秘人可以接任务了
        this->missionStatus = true;
        cout << "有新的任务可接" << endl;
        system("pause");
        system("cls");
        return true;
    }
    // 蜥蜴人对话
    mission = player.getMission(10);
    if (this->id == "NN-13" && mission != nullptr && !mission->isFinished){
        mission->missionFinish(player);
        cout << this->talkContent[10].end << endl;
        string t ="你：我想要回到原来的世界，怎么办\n蜥蜴人祭司：我这里有6个神珠，据说集齐7个就可以实现你的愿望了，但最后一个我们丢失了它。\n你：足够了，我会寻找这最后一个的";
        Scene::show(t);
        t = "从下面前往魔王城吧, 巨龙应该还在沉睡";
        Scene::show(t);
        system("pause");
        system("cls");
        return true;
    }
    // 公主的委托
    if (this->id == "NN-15"){
        Mission *mission1 = player.getMission(7);
        if (mission1 != nullptr && mission1->isFinished){
            mission1 = player.getMission(10);
            if (mission1 != nullptr && mission1->isFinished){
                // 公主可接任务
                globalNPC[13].missionStatus = true;
                return false;
            }
            else{
                string t = "你需要去了解真相";
                Scene::show(t);
            }
        }
        else{
            string t = "你快离开吧, 这里很危险";
            Scene::show(t);
        }
    }
    return false;
}

