#include "pch.h"
#include "CppUnitTest.h"
#include "../header/utils/Config.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using json = nlohmann::json;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			try {
				//Config config("C:\\Users\\郭\\source\\repos\\moAAjDdgm1Maa\\MiniWar\\config.json");
				Config config = Config::getInstance();
				json j = config.getConfig({ "mapSize", "large", "width"});
				int res = j.template get<int>();
				Assert::AreEqual(res, 64);
			}
			catch (std::runtime_error& e) {
				Assert::AreEqual(e.what(), "");
			}
			//Config config("../../MiniWar/config.json");
			//json j = config.getConfig({ "mapSize", "large", "width"});
			//int res = j.template get<int>();
			//Assert::AreEqual(res, 64);
		}
	};
}
