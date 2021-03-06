//
// Created by Leo on 2019-08-30.
//

#include "Item.h"

#include <map>
#include <fstream>

#include "Status.h"
#include "Tool.h"
#include "global.h"
#include "templateHeader.h"
/*
 * @brief 无参的构造函数 用于物品栏的占位 即为空
 */
Item::Item() {
    this->id = 0;
    this->nameCN = "";
    this->nameEN = "";
    this->description = "";
    this->num = 0;
    this->boughtPrice = 0;
    this->canDrop = false;
    this->canSell = false;
}

/*
 * @brief 从文件中读取出 Item 类的基本信息 构造相应的对象
 *
 * @param id Item 对象对应的表示 id
 * @param number 对象物品的数量
 * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件中物品的 id 对应:
 * 001 -- 099: 武器 Weapon
 * 101 -- 199: 防具 Armor
 * 201 -- 299: 药剂 Drug
 * 301 -- 399: 任务道具
 *
 * items.txt 文件内容 (大致格式):
 * * * * * * * * * * * * * * * * * * * * * * * * *
 * id 1
 * nameCN 剑
 * nameEN sword
 * description 一把普通的剑，好像是量产的类型
 * boughtPrice 10
 * canDrop true
 * canSell true
 *
 * 分别对应 Item 类的各个属性
 * 所有键值对必须存在
 * * * * * * * * * * * * * * * * * * * * * * * * *
 */
Item::Item(int id, int number) {
    ifstream f(ITEMS_TXT_PATH);
    string str;

    // 找到对应 id 处
    while (getline(f, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && fromString<int>(idLine[1]) == id) {
                break;
            }
        }
    }
    // 将 对应 id 行到下一个空行之间的内容读取为键值对
    map<string, string> data = Tool::dataMap(f);

    this->id = id;
    this->nameCN = data["nameCN"];
    this->nameEN = data["nameEN"];
    this->description = data["description"];
    this->num = number;
    this->boughtPrice = fromString<int>(data["boughtPrice"]);
    this->canDrop = Tool::boolFromString(data["canDrop"]);
    this->canSell = Tool::boolFromString(data["canSell"]);

    f.close();
}

/*
 * @brief 打印物品的介绍
 */
void Item::showDescription() {
    // 剔除 占位 Item 对象，输出为无
    if (id != 0) {
        cout << "中文名:\t" << this->nameCN << endl;
        cout << "英文名:\t" << this->nameEN << endl;
        cout << "简介:\t" << this->description << endl;
        cout << "数量:\t" << this->num << endl;
        cout << "价值:\t" << this->boughtPrice << endl;

        if (this->canDrop) {
            cout << "属性:\t" << "可丢弃  ";
        } else {
            cout << "属性:\t" << "不可丢弃";
        }

        if (this->canSell) {
            cout << "可交易  " << endl;
        } else {
            cout << "不可交易" << endl;
        }

    } else {
        cout << "无" << endl;
    }
}

/*
 * @brief 无参的构造函数 即为空
 */
Drug::Drug() : Item(){
    this->HP = 0;
    this->MP = 0;
    this->Phy = 0;
    this->playerTarget = false;
}

/*
 * @brief 从文件中读取出 Drug 类的基本信息 构造相应的对象
 *
 * @param id Item 超类对应的 id (id 范围: 201 -- 299)
 * @param number 对象物品的数量
 *
 * drug.txt 文件内容 (大致格式):
 * * * * * * * * * * * * * * * * * * * * * * * * *
 * id
 * buff
 * HP
 * MP
 * Phy
 * playerTarget
 *
 * 分别对应 Drug 类的各个属性
 * 所有键值对必须存在
 * * * * * * * * * * * * * * * * * * * * * * * * *
 */
Drug::Drug(int id, int number) : Item(id, number){
    ifstream f(DRUG_TXT_PATH);
    string str;

    // 找到对应 id 处
    while (getline(f, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && fromString<int>(idLine[1]) == id) {
                break;
            }
        }
    }
    // 将 对应 id 行到下一个空行之间的内容读取为键值对
    map<string, string> data = Tool::dataMap(f);

    this->HP = fromString<int>(data["HP"]);
    this->MP = fromString<int>(data["MP"]);
    this->Phy = fromString<int>(data["Phy"]);
    this->playerTarget = Tool::boolFromString(data["playerTarget"]);

    f.close();
}
/*
 * @brief 展示药品的信息
 */
void Drug::showDescription(){
    Item::showDescription();
    cout << "回复血量:\t" << this->HP << endl;
    cout << "回复蓝量:\t" << this->MP << endl;
    cout << "回复体力值:\t" <<  this->Phy << endl;
    if (this->playerTarget) {
        cout << "使用方式:\t" << "对己" << endl;
    } else {
        cout << "使用方式:\t" << "对敌" << endl;
    }
}

/*
 * @brief 武器的默认构造函数
 * 构造一个空的武器,id为0
 */
Weapon::Weapon() : Item() {
    this->ATK = 0;
    this->DEF = 0;
    this->Speed = 0;
}

/*
 * @brief 从文件中读取出 Weapon 类的基本信息 构造相应的对象
 *
 * @param id Item 超类对应的 id (id 范围: 001 -- 099)
 * @param number 对象物品的数量
 *
 * weapon.txt 文件内容 (大致格式):
 * * * * * * * * * * * * * * * * * * * * * * * * *
 * id
 * ATK
 * DEF
 * Speed
 *
 * 分别对应 Weapon 类的各个属性
 * 所有键值对必须存在
 * * * * * * * * * * * * * * * * * * * * * * * * *
 */
Weapon::Weapon(int id, int number) : Item(id, number) {
    ifstream f(WEAPON_TXT_PATH);
    string str;

    // 找到对应 id 处
    while (getline(f, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && fromString<int>(idLine[1]) == id) {
                break;
            }
        }
    }
    // 将 对应 id 行到下一个空行之间的内容读取为键值对
    map<string, string> data = Tool::dataMap(f);

    this->ATK = fromString<int>(data["ATK"]);
    this->DEF = fromString<int>(data["DEF"]);
    this->Speed = fromString<int>(data["Speed"]);

    f.close();
}

/*
 * @brief 展示武器的信息
 */
void Weapon::showDescription() {
    Item::showDescription();
    cout << "攻击:" << this->ATK << endl;
    cout << "防御:" << this->DEF << endl;
    cout << "速度:" << this->Speed << endl;
}

/*
 * @brief 无参的构造函数 即为空
 */
Armor::Armor() : Item(){
    this->ATK = 0;
    this->DEF = 0;
    this->Speed = 0;
}

/*
 * @brief 从文件中读取出 Armor 类的基本信息 构造相应的对象
 *
 * @param id Item 超类对应的 id (id 范围: 101 -- 199)
 * @param number 对象物品的数量
 *
 * armor.txt 文件内容 (大致格式):
 * * * * * * * * * * * * * * * * * * * * * * * * *
 * id
 * ATK
 * DEF
 * Speed
 *
 * 分别对应 Armor 类的各个属性
 * 所有键值对必须存在
 * * * * * * * * * * * * * * * * * * * * * * * * *
 */
Armor::Armor(int id, int number) : Item(id, number) {
    ifstream f(ARMOR_TXT_PATH);
    string str;

    // 找到对应 id 处
    while (getline(f, str)) {
        if (!str.empty()) {
            vector<string> idLine = Tool::split(str);
            if (idLine[0] == "id" && fromString<int>(idLine[1]) == id) {
                break;
            }
        }
    }
    // 将 对应 id 行到下一个空行之间的内容读取为键值对
    map<string, string> data = Tool::dataMap(f);

    this->ATK = fromString<int>(data["ATK"]);
    this->DEF = fromString<int>(data["DEF"]);
    this->Speed = fromString<int>(data["Speed"]);

    f.close();
}

/*
 * 展示防具的描述信息
 */
void Armor::showDescription() {
    Item::showDescription();
    cout << "攻击:" << this->ATK << endl;
    cout << "防御:" << this->DEF << endl;
    cout << "速度:" << this->Speed << endl;
}
