/**
* @file DepthOfField.frag
*/
#version 430

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D texColorArray[2]; // 0:カラー 1:深度

/*
*画面の情報
*
*x: 1.0 / ウィンドウの幅(ピクセル単位)
*y: 1.0 / ウィンドウの高さ(ピクセル単位)
*x: near(m単位)
*y: far(m単位)
*/
uniform vec4 viewInfo;

/**
*
*x: 焦平面(ピントの合う位置のレンズからの距離. mm単位)
*y: 焦点距離(光が1点に集まる位置のレンズからの距離. mm単位)
*z: 開口(光の取入口のサイズ. mm単位)
*w: センサーサイズ(光を受けるセンサーの横幅. mm単位)
*/
uniform vec4 cameraInfo;

//ボアソンディスクサンプリングのための座標リスト
const int poissonSampleCount = 12;
const vec2 poissonDisk[poissonSampleCount] = 
{
	{ -0.326, -0.406 }, { -0.840, -0.074 }, { -0.696,  0.457 },
	{ -0.203,  0.621 }, {  0.962, -0.195 }, {  0.473, -0.480 },
	{  0.519,  0.767 }, {  0.185, -0.893 }, {  0.507,  0.064 },
	{  0.896,  0.412 }, { -0.322, -0.933 }, { -0.792, -0.598 }
};

/*
深度バッファの深度値をビュー座標系の深度値に変換する

@param w 深度バッファの深度値

@return wをビュー座標系の深度値に変換した値
*/
float ToRealZ(float w)
{
	float near = viewInfo.z;
	float far = viewInfo.w;
	float n = 2.0 * w - 1.0;
	return -2.0 * near * far / (far + near - n * (far - near));
}

/*
錯乱円の半径を計算する

@param objectDistance 対象ピクセルのカメラ座標系におけるZ座標

@return 錯乱円の半径(ピクセル単位)
*/
vec2 CalcCoc(float objectDistance)
{
	float focalPlane = cameraInfo.x;
	float focalLength = cameraInfo.y;
	float apertrue = cameraInfo.z;
	float sensorSize = cameraInfo.w;
	float mmToPixel = 1.0 / (sensorSize * viewInfo.x);

	objectDistance *= -1000.0; // mm単位に変換し、値を正にする
	return abs(apertrue * (focalLength * (focalPlane - objectDistance)) /
		(objectDistance * (focalPlane - focalLength))) * mmToPixel * viewInfo.xy;
}

/*
* 被写界深度シェーダー
*/
void main()
{
	float centerZ = ToRealZ(texture(texColorArray[1], inTexCoord).r);
	vec2 coc = CalcCoc(centerZ);
	fragColor.rgb = texture(texColorArray[0], inTexCoord).rgb;
	for(int i = 0; i < poissonSampleCount; ++i)
	{
		vec2 uv = inTexCoord + coc * poissonDisk[i];
		fragColor.rgb += texture(texColorArray[0], uv).rgb;
	}
	fragColor.rgb = fragColor.rgb / float(poissonSampleCount + 1);
	fragColor.a = 1.0;
}