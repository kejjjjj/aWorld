#pragma once

struct GfxStaticModelDrawInst;
GfxStaticModelDrawInst* GetStaticModelDrawInstPointer();

void CM_AddModel(const GfxStaticModelDrawInst* model);
struct cm_model CM_MakeModel(const GfxStaticModelDrawInst* model);
