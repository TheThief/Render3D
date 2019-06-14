#pragma once

template<typename PositionType = Vector4>
struct alignstruct(16) position_vertex
{
	PositionType Position;

	position_vertex()
	{
	}

	position_vertex(const PositionType& Position) :
		Position(Position)
	{
	}

	forceinline position_vertex operator -(const position_vertex& rhs) const
	{
		position_vertex Result;
		Result.Position = Position - rhs.Position;
		return Result;
	}

	forceinline position_vertex operator +(const position_vertex& rhs) const
	{
		position_vertex Result;
		Result.Position = Position + rhs.Position;
		return Result;
	}

	forceinline position_vertex operator *(float w) const
	{
		position_vertex Result;
		Result.Position = Position * w;
		return Result;
	}
};
