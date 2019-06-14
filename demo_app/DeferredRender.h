#pragma once

#include "deferred_point_light_mesh.h"

using DeferredLightVertex = position_vertex<Vector4>;

struct alignstruct(16) DeferredLightVertexInterpolants : public DeferredLightVertex
{
	forceinline DeferredLightVertexInterpolants operator -(const DeferredLightVertexInterpolants& rhs) const
	{
		DeferredLightVertexInterpolants Result;
		Result.Position = Position - rhs.Position;
		return Result;
	}

	forceinline DeferredLightVertexInterpolants operator +(const DeferredLightVertexInterpolants& rhs) const
	{
		DeferredLightVertexInterpolants Result;
		Result.Position = Position + rhs.Position;
		return Result;
	}

	forceinline DeferredLightVertexInterpolants operator *(float w) const
	{
		DeferredLightVertexInterpolants Result;
		Result.Position = Position * w;
		return Result;
	}
};

struct PixelGlobals_Deferred_InitialPass
{
	Array2d<ARGB>& Texture;
};

struct PixelGlobals_Deferred_LightingPass
{
	Array2d<ARGB>& Diffuse;
	Array2d<Vector3>& WorldPosition;
	Array2d<Vector3>& Normal;
	Vector3 LightPosition;
	float LightIntensity;
};

DeferredLightVertexInterpolants DeferredLightVertexVertexFunc(const VertexGlobals& g, const DeferredLightVertex& v)
{
	DeferredLightVertexInterpolants Result;
	Result.Position = v.Position * g.ObjectTransform * g.ViewProjection;
	return Result;
}

struct DepthComparator_Deferred_Light
{
	static forceinline bool Compare(const float& PixelDepth, const float& BufferDepth)
	{
		return /*(BufferDepth != FLT_MAX) &&*/ (PixelDepth > BufferDepth);
	}

	static const bool bSetDepth = false;
};

struct TextureVertexPixelFunc_Deferred_InitialPass
{
	static forceinline void Func(const PixelGlobals_Deferred_InitialPass& g, const TextureVertexInterpolants& v, ARGB& Diffuse, Vector3& WorldPosition, Vector3& Normal)
	{
		fRGBA fColour = g.Texture.Sample<fRGBA>(v.TextureCoords.x * g.Texture.GetWidth() - 0.5f, v.TextureCoords.y * g.Texture.GetHeight() - 0.5f);
		Diffuse = (ARGB)fColour;

		WorldPosition = v.WorldPosition;
		Normal = v.Normal;
	}
};

struct DeferredLightVertexPixelFunc_Deferred_LightingPass
{
	static forceinline void Func(const PixelGlobals_Deferred_LightingPass& g, const DeferredLightVertexInterpolants& v, ARGB& Colour)
	{
		int ScreenX = RoundToInt(v.Position.x);
		int ScreenY = RoundToInt(v.Position.y);
		fRGBA fColour = g.Diffuse(ScreenX, ScreenY);
		Vector3 Normal = g.Normal(ScreenX, ScreenY);
		Vector3 WorldPosition = g.WorldPosition(ScreenX, ScreenY);

		Vector3 ToLight = g.LightPosition - WorldPosition;
		fColour *= g.LightIntensity * Saturate(ToLight.Normalized().Dot(Normal)) / ToLight.SizeSquared();
		Colour = (ARGB)fColour;
	}
};
