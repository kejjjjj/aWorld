#pragma once

struct GfxViewParms;

void RB_DrawDebug(GfxViewParms* viewParms);

void CM_DrawCollisionPoly(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_DrawCollisionEdges(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_ShowCollision(GfxViewParms* viewParms);

void CM_DrawBrushBounds(const cbrush_t* brush, bool depthTest);
