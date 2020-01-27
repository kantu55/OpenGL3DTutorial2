/*
@file Font.h
*/
#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED
#include <GL/glew.h>
#include "Sprite.h"
#include <glm/glm.hpp>
#include<vector>
#include<string>


/*
ビットマップフォント描画クラス
*/
class FontRenderer
{
public:
	FontRenderer() = default;
	~FontRenderer() = default;

	/*
	deleteを指定することで、その型の変数のコピーを禁止する
	*/
	FontRenderer(const FontRenderer&) = delete;
	FontRenderer& operator=(const FontRenderer&) = delete;

	bool Init(size_t maxChar);                //フォント描画クラスを初期化
	bool LoadFromFile(const char* path);      //フォント情報ファイルとテクスチャを読み込む
	void BeginUpdate();                       //文字の追加を開始
	bool AddString(const glm::vec2&, const wchar_t*);  //必要なだけ実行して文字を追加
	void EndUpdate();                         //追加を終了
	void Draw(const glm::vec2&) const;        //フォントを描画
	void Color(const glm::vec4&) const;       //色を変更する
	float LineHeight() const;                 //フォントの高さを返す

private:
	SpriteRenderer spriteRenderer;  //描画に使用するスプライトレンダラー
	std::vector<Texture::Image2DPtr> textures;  //フォントのテクスチャリスト
	float lineHeight = 0;  //行の高さ
	float base = 0;  //行の上部からベースラインまでの距離

	//文字情報
	struct CharacterInfo
	{
		int id = -1;  //文字コード
		int page = 0; //文字が含まれるフォントテクスチャの番号
		glm::vec2 uv = glm::vec2(0);   //文字のテクスチャ座標
		glm::vec2 size = glm::vec2(0); //文字の表示サイズ
		glm::vec2 offset = glm::vec2(0); //表示位置の補正値
		float xadvance = 0; //次の文字を表示する位置
	};
	std::vector<CharacterInfo> characterInfoList; //文字情報リスト
};

#endif // FONT_H_INCLUDED