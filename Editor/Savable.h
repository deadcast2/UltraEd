#pragma once

#include <map>
#include "deps/DXSDK/include/d3d8.h"
#include "vendor/cJSON.h"

using namespace std;

namespace UltraEd
{
	struct SavableType
	{
		enum Value { View, Actor };
	};

	typedef struct
	{
		cJSON *object;
		SavableType::Value type;
	} Savable;

	class CSavable
	{
	public:
		virtual Savable Save() = 0;
		virtual bool Load(IDirect3DDevice8 *device, cJSON *root) = 0;
		map<string, string> GetResources() { return resources; };

	protected:
		map<string, string> resources;
	};
}
