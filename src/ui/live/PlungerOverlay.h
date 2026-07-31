// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

class Plunger;

class PlungerOverlay final
{
public:
   PlungerOverlay() = default;
   ~PlungerOverlay() = default;

   void SetUIScale(float scale) { m_uiScale = scale; }

   void Update();

private:
   const Plunger *GetPlunger();
   void DrawBar(const ImVec2 &trackMin, const ImVec2 &trackMax, float pos, float fade) const;
   void DrawTicks(const ImVec2 &trackMin, const ImVec2 &trackMax, float tickGap, float tickLen, float fade) const;

   float m_uiScale = 1.0f;
   const Plunger *m_plunger = nullptr;
   bool m_searched = false;
   float m_idleTime = FLT_MAX;
};
