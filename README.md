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

