#include "cm_model.hpp"
#include "cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include <com/com_vector.hpp>
#include <iomanip>

void CM_AddModel(const GfxStaticModelDrawInst* model)
{

	cm_model xmodel
	(
		model->model->name,
		model->placement.origin,
		AxisToAngles((fvec3(*)[3])model->placement.axis),
		model->placement.scale
	);

	CClipMap::Insert(std::make_unique<cm_model>(xmodel));
}


int cm_model::map_export(std::stringstream& o, int index)
{

	o << "// entity " << index << '\n';
	o << "{\n";
	o << std::quoted("angles") << ' ' << std::quoted(std::format("{} {} {}", angles.x, angles.y, angles.z)) << '\n';
	o << std::quoted("modelscale") << ' ' << std::quoted(std::format("{}", modelscale)) << '\n';
	o << std::quoted("origin") << ' ' << std::quoted(std::format("{} {} {}", origin.x, origin.y, origin.z)) << '\n';
	o << std::quoted("model") << ' ' << std::quoted(name) << '\n';
	o << std::quoted("classname") << ' ' << std::quoted("misc_model") << '\n';
	o << "}\n";

	return ++index;

}