#include "pch.h"
#include "CppUnitTest.h"
#include "../header/utils/Config.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Config config("../MiniWar/config.json");
			json j = config.getConfig({ "mapSize", "large", "width"});
			int res = j.template get<int>();
			Assert::AreEqual(res, 64);
		}
	};
}
