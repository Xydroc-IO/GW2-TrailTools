#include "TrailToolsTrailGeom.h"

#include <algorithm>
#include <cmath>

bool TrailToolsTrailGeom::IsBreak(const PathingTrails::WorldPoint& p)
{
	return p.x == 0.f && p.y == 0.f && p.z == 0.f;
}

void TrailToolsTrailGeom::ClearSelection(std::vector<int>& selected)
{
	selected.clear();
}

bool TrailToolsTrailGeom::IsSelected(const std::vector<int>& selected, int idx)
{
	return std::find(selected.begin(), selected.end(), idx) != selected.end();
}

void TrailToolsTrailGeom::ToggleSelect(std::vector<int>& selected, int idx)
{
	auto it = std::find(selected.begin(), selected.end(), idx);
	if (it != selected.end())
		selected.erase(it);
	else
		selected.push_back(idx);
}

void TrailToolsTrailGeom::SelectRange(std::vector<int>& selected, int a, int b)
{
	if (a > b)
		std::swap(a, b);
	selected.clear();
	for (int i = a; i <= b; ++i)
		selected.push_back(i);
}

void TrailToolsTrailGeom::Reverse(TrailToolsDetail::DraftTrail& trail)
{
	std::reverse(trail.points.begin(), trail.points.end());
}

void TrailToolsTrailGeom::Densify(TrailToolsDetail::DraftTrail& trail, float maxSpacing)
{
	if (maxSpacing < 0.25f)
		maxSpacing = 0.25f;
	std::vector<PathingTrails::WorldPoint> out;
	out.reserve(trail.points.size() * 2);
	for (size_t i = 0; i < trail.points.size(); ++i)
	{
		out.push_back(trail.points[i]);
		if (i + 1 >= trail.points.size())
			break;
		const auto& a = trail.points[i];
		const auto& b = trail.points[i + 1];
		if (IsBreak(a) || IsBreak(b))
			continue;
		const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
		const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
		if (dist <= maxSpacing)
			continue;
		const int n = static_cast<int>(std::floor(dist / maxSpacing));
		for (int k = 1; k <= n; ++k)
		{
			const float t = static_cast<float>(k) / static_cast<float>(n + 1);
			out.push_back({ a.x + dx * t, a.y + dy * t, a.z + dz * t });
		}
	}
	trail.points = std::move(out);
}

void TrailToolsTrailGeom::Smooth(TrailToolsDetail::DraftTrail& trail, int passes)
{
	if (passes < 1)
		passes = 1;
	if (passes > 2)
		passes = 2;
	for (int pass = 0; pass < passes; ++pass)
	{
		std::vector<PathingTrails::WorldPoint> out;
		out.reserve(trail.points.size() * 2);
		size_t i = 0;
		while (i < trail.points.size())
		{
			if (IsBreak(trail.points[i]))
			{
				out.push_back(trail.points[i]);
				++i;
				continue;
			}
			size_t j = i;
			while (j < trail.points.size() && !IsBreak(trail.points[j]))
				++j;
			const size_t n = j - i;
			if (n < 2)
			{
				for (size_t k = i; k < j; ++k)
					out.push_back(trail.points[k]);
			}
			else
			{
				out.push_back(trail.points[i]);
				for (size_t k = i; k + 1 < j; ++k)
				{
					const auto& a = trail.points[k];
					const auto& b = trail.points[k + 1];
					out.push_back({
						a.x * 0.75f + b.x * 0.25f,
						a.y * 0.75f + b.y * 0.25f,
						a.z * 0.75f + b.z * 0.25f });
					out.push_back({
						a.x * 0.25f + b.x * 0.75f,
						a.y * 0.25f + b.y * 0.75f,
						a.z * 0.25f + b.z * 0.75f });
				}
				out.push_back(trail.points[j - 1]);
			}
			i = j;
		}
		trail.points = std::move(out);
	}
}

void TrailToolsTrailGeom::DeleteIndices(TrailToolsDetail::DraftTrail& trail,
	std::vector<int>& selected, int& primarySel)
{
	if (selected.empty())
		return;
	std::sort(selected.begin(), selected.end());
	selected.erase(std::unique(selected.begin(), selected.end()), selected.end());
	for (int i = static_cast<int>(selected.size()) - 1; i >= 0; --i)
	{
		const int idx = selected[static_cast<size_t>(i)];
		if (idx < 0 || idx >= static_cast<int>(trail.points.size()))
			continue;
		trail.points.erase(trail.points.begin() + idx);
	}
	selected.clear();
	if (primarySel >= static_cast<int>(trail.points.size()))
		primarySel = static_cast<int>(trail.points.size()) - 1;
}
