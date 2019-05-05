#include "Unit.h"
#include "../Editor/Util.h"

using namespace UltraEd;

int main()
{
	CUnit testRunner;

	testRunner.It("creates a new resource name with number", [](CAssert assert) {
		assert.Equal(CUtil::NewResourceName(26), "UER_26");
	});

	testRunner.Run();

	return 0;
}
