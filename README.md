# MiniWar

# 类
## 地图
地图是一个二维数组，由索引代表各个地块
## 地块
地块需要血量属性、保存的武器列表
HP: int
Weapons: List
需要具有进攻、拦截命令
Attack(x:int,y:int)
Intercept(weapon:Weapon)
## 武器
武器需要拦截的概率、类型
Interceptable: float
Type: int
## 资源库
规定五种基本资源：黄金、石油、钢材、电力、人力
Gold: int
Oil: int
Electricity: int
Steel: int
Labor: int
这些属性本身不会随时间增加，而是由资源耗费进行交换
## 建筑
建筑通用两个属性：等级、升级耗费。
产出效率根据等级划分。
一个地块最多拥有一个建筑
### 发电厂
### 炼油厂
### 军事工厂
### 民生工厂
### 炼钢厂
### 研究所


{
	"mapSize": {
		"large": {
			"width": 64,
			"height":64 
			"aiCount": 1
		},
		"medium": {
			"width": 32,
			"height": 32,
			"aiCount": 1
		},
		"small": {
			"width": 16,
			"height": 16,
			"aiCount": 1
		}
	},
	"Weapon": {
		0: {
			"name": "CM",
			"damage": 25,
            "damageRange": 0,
            "attackSpeed": 0.0625,
            "AttackRange": [0.0, 0.25],
            "cost": [10000, 200, 300, 1000, 50]
		},	
        1: {
            "name": "MRBM",
            "damage": 74,
            "damageRange": 1.2,
            "attackSpeed": 0.03125,
            "AttackRange": [0.0, 0.5],
            "cost": [35000, 500, 700, 1500, 75]
        },
        2: {
            "name": "ICBM",
            "damage": 45,
            "damageRange": 1.5,
            "attackSpeed": 0.025,
            "AttackRange": [0.2, 0.8],
            "cost": [50000, 1000, 1500, 2000, 100]
        }
	},
    "Region": {
        "hp": [200, 300],
        "Army": [30, 50],
        "OriginSize": [1.5, 2.5],
        "CapitalHp": 1000,
        "CapitalArmy": 200
    },
    "Building": {
        "PowerStation": {
            "BuildCost": [3000, 500, 1000, 200, 30],
            "Product": [0, 0, 0, 5, 0],
            "UpLevelFactor": [1.3, 1.5],
            "UpLevelCost1": [30000, 2000, 5000, 1000, 20]
            "UpLevelCost2": [50000, 5000, 10000, 2000, 40]
        },
        "Refinery": {
            "BuildCost": [5000, 1000, 2000, 300, 50],
            "Product": [0, 15, 0, 0, 0],
            "UpLevelFactor": [1.3, 1.5],
            "UpLevelCost1": [50000, 5000, 10000, 2000, 40]
            "UpLevelCost2": [80000, 10000, 20000, 4000, 60]
        },
        "SteelFactory": {
            "BuildCost": [5000, 1000, 2000, 300, 50],
            "Product": [0, 0, 20, 0, 0],
            "UpLevelFactor": [1.3, 1.5],
            "UpLevelCost1": [50000, 5000, 10000, 2000, 40]
            "UpLevelCost2": [80000, 10000, 20000, 4000, 60]
        },
        "CivilFactory": {
            "BuildCost": [5000, 1000, 2000, 300, 50],
            "Product": [500, 0, 0, 0, 0],
            "UpLevelFactor": [1.3, 1.5],
            "UpLevelCost1": [50000, 5000, 10000, 2000, 40]
            "UpLevelCost2": [80000, 10000, 20000, 4000, 60]
        },
        "MilitaryFactory": {
            "BuildCost": [5000, 1000, 2000, 300, 50],
            "CD": [15, 10, 7],
            "UpLevelCost1": [50000, 5000, 10000, 2000, 40]
            "UpLevelCost2": [80000, 10000, 20000, 4000, 60]
        },
    },
    "ReasearchInstitution":{
        "BuildCost": 200000,
        "OUpLevelCost": {
            "PowerStation": [100000, 300000],
            "Refinery": [100000, 300000],
            "SteelFactory": [100000, 300000],
            "CivilFactory": [300000, 500000],
            "MilitaryFactory": [300000, 500000],
            0: [200000, 400000],
            1: [250000, 500000],
            2: [300000, 600000]
        }
    }
}

