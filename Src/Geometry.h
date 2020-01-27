#pragma once

/*
@file Geometry.h
*/
#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED
#include <GL/glew.h>

//2Dベクトル型
struct Vector2
{
	float x, y;
};

//3Dベクトル型
struct Vector3
{
	float x, y, z;
};

//RGBAカラー型
struct Color
{
	float r, g, b, a;
};

//頂点データ型
struct Vertex
{
	Vector3 position;
	Color color;
	Vector2 texCoord; //テクスチャの座標
	Vector3 normal;
};

/*
ポリゴン表示単位
*/
struct Mesh
{
	GLenum mode; //プリミティブの種類
	GLsizei count; //描画するインデックス数
	const GLvoid* indices; //描画開始インディアンスのバイトオフセット
	GLint baseVertex; //インデックス0番とみなされる頂点配列内の位置
};

#endif // !GEOMETRY_H_INCLUDED
