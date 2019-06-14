
struct alignstruct(16) TextureVertex : public position_vertex<Vector4>
{
	Vector3 Normal;
	Vector2 TextureCoords;

	TextureVertex()
	{
	}

	TextureVertex(const Vector4& Position, const Vector4& Normal, const Vector2& TextureCoords) :
		position_vertex<Vector4>(Position),
		Normal(Normal),
		TextureCoords(TextureCoords)
	{
	}
};

struct alignstruct(16) TextureVertexInterpolants : public TextureVertex
{
	Vector3 WorldPosition;

	forceinline TextureVertexInterpolants operator -(const TextureVertexInterpolants& rhs) const
	{
		TextureVertexInterpolants Result;
		Result.Position = Position - rhs.Position;
		Result.WorldPosition = WorldPosition - rhs.WorldPosition;
		Result.Normal = Normal - rhs.Normal;
		Result.TextureCoords = TextureCoords - rhs.TextureCoords;
		return Result;
	}

	forceinline TextureVertexInterpolants operator +(const TextureVertexInterpolants& rhs) const
	{
		TextureVertexInterpolants Result;
		Result.Position = Position + rhs.Position;
		Result.WorldPosition = WorldPosition + rhs.WorldPosition;
		Result.Normal = Normal + rhs.Normal;
		Result.TextureCoords = TextureCoords + rhs.TextureCoords;
		return Result;
	}

	forceinline TextureVertexInterpolants operator *(float w) const
	{
		TextureVertexInterpolants Result;
		Result.Position = Position * w;
		Result.WorldPosition = WorldPosition * w;
		Result.Normal = Normal * w;
		Result.TextureCoords = TextureCoords * w;
		return Result;
	}
};

struct PixelGlobals
{
	Array2d<ARGB>& Texture;
	Vector3 LightPosition;
	float LightIntensity;
};

struct VertexGlobals
{
	Matrix ObjectTransform;
	Matrix ViewProjection;
};

TextureVertexInterpolants TextureVertexVertexFunc(const VertexGlobals& g, const TextureVertex& v)
{
	TextureVertexInterpolants Result;
	Result.WorldPosition = v.Position * g.ObjectTransform;
	Result.Position = Result.WorldPosition * g.ViewProjection;
	Result.TextureCoords = v.TextureCoords;
	Result.Normal = (Vector4(v.Normal, 0) * g.ObjectTransform).Normalized();
	return Result;
}

struct DepthComparator_Less_Set
{
	static forceinline bool Compare(const float& PixelDepth, const float& BufferDepth)
	{
		return (PixelDepth < BufferDepth);
	}

	static const bool bSetDepth = true;
};

struct DepthComparator_Greater_Set
{
	static forceinline bool Compare(const float& PixelDepth, const float& BufferDepth)
	{
		return (PixelDepth > BufferDepth);
	}

	static const bool bSetDepth = true;
};

struct DepthComparator_Equal_NoSet
{
	static forceinline bool Compare(const float& PixelDepth, const float& BufferDepth)
	{
		return (PixelDepth == BufferDepth);
	}

	static const bool bSetDepth = false;
};

struct TextureVertexPixelFunc_ColourPass
{
	static forceinline void Func(const PixelGlobals& g, const TextureVertexInterpolants& v, ARGB& Colour)
	{
		//Vector2 UV = v.TextureCoords * Vector2((float)g.Texture.GetWidth(), (float)g.Texture.GetHeight()) - Vector2(0.5f, 0.5f);
		float UVx = v.TextureCoords.x * (float)(g.Texture.GetWidth() - 1);
		float UVy = v.TextureCoords.y * (float)(g.Texture.GetHeight() - 1);
		Vector3 ToLight = g.LightPosition - v.WorldPosition;
		float Light = g.LightIntensity * Saturate(ToLight.Normalized().Dot(v.Normal)) / ToLight.SizeSquared();
		//fARGB fColour = fARGB(0.7f, 0.7f, 0.7f);
		fRGBA fColour = g.Texture.Sample<fRGBA>(UVx, UVy);
		//fARGB fColour = g.Texture.SampleWrap<fARGB>(UVx, UVy);
		//fARGB fColour = g.Texture.Get(TruncateToInt(UVx), TruncateToInt(UVy));
		fColour *= Light;
		Colour = (ARGB)fColour;
	}
};

struct TextureVertexPixelFunc_Unlit
{
	static forceinline void Func(const PixelGlobals& g, const TextureVertexInterpolants& v, ARGB& Colour)
	{
		float UVx = v.TextureCoords.x * (float)(g.Texture.GetWidth() - 1);
		float UVy = v.TextureCoords.y * (float)(g.Texture.GetHeight() - 1);
		fRGBA fColour = g.Texture.Sample<fRGBA>(UVx, UVy);
		Colour = (ARGB)fColour;
	}
};
