// license:GPLv3+

#include "core/stdafx.h"
#include "PlungerOverlay.h"

#include "imgui/imgui.h"
#include "parts/plunger.h"
#include "renderer/Renderer.h"
#include "ui/live/LiveUI.h"

static constexpr float fadeOutDelay = 1.f;
static constexpr float fadeOutDuration = 1.f;

const Plunger *PlungerOverlay::GetPlunger()
{
   // Resolve (once) the first plunger of the table. The parts list is stable during
   // play, but the plunger may not be simulated, in which case there is nothing to show.
   if (!m_searched)
   {
      m_searched = true;
      for (const IEditable *const part : g_pplayer->m_ptable->GetParts())
      {
         if (part->GetItemType() == eItemPlunger)
         {
            m_plunger = static_cast<const Plunger *>(part);
            break;
         }
      }
   }
   return m_plunger;
}

void PlungerOverlay::Update()
{
   const Plunger *const plunger = GetPlunger();
   if (plunger == nullptr)
      return;

   float pos;
   if (!plunger->GetNormalizedPosition(pos))
      return;

   const auto &io = ImGui::GetIO();

   // Keep the overlay visible while the player operates the plunger, then fade out once it
   // has been left alone for a couple of seconds.
   if (plunger->IsActive())
      m_idleTime = 0.f;
   else
      m_idleTime += io.DeltaTime;
   if (m_idleTime > fadeOutDelay + fadeOutDuration)
      return;

   const float fade = (m_idleTime < fadeOutDelay) ? 1.f : (fadeOutDelay + fadeOutDuration - m_idleTime) / fadeOutDuration;
   const float size = min(200.f * m_uiScale, 0.15f * min(io.DisplaySize.x, io.DisplaySize.y));
   const float offset = 0.15f * size;
   const ImVec2 barSize = ImVec2(size * 0.25f, size);
   const float tickLen = barSize.x * 0.5f;
   const float tickGap = barSize.x * 0.2f;
   // The window has to hold the bar and the tick marks drawn along its left side
   const ImVec2 fullSize = ImVec2(tickLen + tickGap + barSize.x, size);
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
      | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   // Bottom right corner, on the side where the real plunger is, and away from the plumb overlay
   ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - fullSize.x - offset, io.DisplaySize.y - fullSize.y - offset));
   ImGui::SetNextWindowSize(fullSize);
   ImGui::Begin("PlungerOverlay", nullptr, window_flags);
   const ImVec2 &pos0 = ImGui::GetWindowPos();
   // Travel range, aligned to the right edge of the window
   const ImVec2 trackMin = pos0 + ImVec2(fullSize.x - barSize.x, 0.f);
   const ImVec2 trackMax = pos0 + fullSize;
   DrawBar(trackMin, trackMax, pos, fade);
   DrawTicks(trackMin, trackMax, tickGap, tickLen, fade);
   ImGui::End();
}

void PlungerOverlay::DrawBar(const ImVec2 &trackMin, const ImVec2 &trackMax, float pos, float fade) const
{
   const int gb = IsAnaglyphStereoMode(g_pplayer->m_renderer->m_stereo3D) ? 255 : 0; // Use white for anaglyph to avoid retinal rivalry/ghosting
   const ImU32 alphaCol = IM_COL32(255, gb, gb, fade * 255.f);
   const ImU32 backCol = IM_COL32(255, gb, gb, fade * 64.f);
   const float rounding = (trackMax.x - trackMin.x) * 0.5f;
   ImGui::GetWindowDrawList()->AddRectFilled(trackMin, trackMax, backCol, rounding);
   ImGui::GetWindowDrawList()->AddRect(trackMin, trackMax, alphaCol, rounding, 0, 2.f * m_uiScale);
   // Remaining travel, full at rest and draining from the top as the plunger is retracted
   const float y = trackMin.y + pos * (trackMax.y - trackMin.y);
   ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(trackMin.x, y), trackMax, alphaCol, rounding);
}

void PlungerOverlay::DrawTicks(const ImVec2 &trackMin, const ImVec2 &trackMax, float tickGap, float tickLen, float fade) const
{
   // Faint tick marks along the left side, splitting the travel range in 8 equal sections
   const int gb = IsAnaglyphStereoMode(g_pplayer->m_renderer->m_stereo3D) ? 255 : 0;
   const ImU32 tickCol = IM_COL32(255, gb, gb, fade * 160.f);
   const float tickX = trackMin.x - tickGap;
   for (int i = 1; i < 8; i++)
   {
      const float tickY = trackMin.y + static_cast<float>(i) * (trackMax.y - trackMin.y) / 8.f;
      ImGui::GetWindowDrawList()->AddLine(ImVec2(tickX - tickLen, tickY), ImVec2(tickX, tickY), tickCol, 3.f * m_uiScale);
   }
}
