precision highp float; 
varying vec4 position;

void main(void)
{
	float normalizedDistance  = position.z / position.w;
	normalizedDistance = (normalizedDistance + 1.0) / 2.0;

	//normalizedDistance -= normalizedDistance/100.0*0.5;

	normalizedDistance += 0.0005;

	//const vec3 multCoef = vec3(1.0, 256.0, 256.0 * 256.0 );
	//vec3 packedFloat = multCoef * normalizedDistance;
	//gl_FragColor = vec4(fract(packedFloat),1.0) + 0.5/255.0;;	



	const vec4 packFactors = vec4( 256.0 * 256.0 * 256.0,    256.0 * 256.0  ,    256.0       ,    1.0);
	const vec4 bitMask     = vec4(0.0                    , 1.0 / 256.0       ,  1.0 / 256.0   , 1.0 / 256.0);
	vec4 packedValue = vec4(fract(packFactors*normalizedDistance));
	packedValue -= packedValue.xxyz * bitMask;
	gl_FragColor  = packedValue;
}


/*
Here's an example. Let's say your input number of 0.431211921. You want to encode that into an RGBA (8 bits integer per channel) pixel.

First you multiply the number by the encoding factors:
(0.431211921, 0.431211921, 0.431211921, 0.431211921) *
(1.0, 256.0, 65536.0, 16777216.0) =
(0.431211921, 110.3902518, 28259.90445, 7234535.54)

Next, you apply the FRC instruction, you get:
(0.431211921, 0.3902518, 0.90445, 0.54).

At this point, those values get stored in your integer RGBA pixel, so they get rounded to 8 bits:
(110/256, 99/256, 231/256, 138/256).

So if you output to the color buffer the value of the pixel, you'll end up with color (110, 99, 231, 138).

Now, for decoding, you get your original pixel value and perform a DP4 with the decoding coefficients:
(110/256, 99/256, 231/256, 138/256) ^
(1.0, 0.00390625, 0.0000152587890625, 0.000000059604644775390625) =
0.4296875 + 0.00151062 + 0.000013767 + 0.000000031 =
0.431211918

... and the original value was 0.431211921

Note that I used my calculator to do the calculations, and it didn't have enough digits for the lower values, so the actual precision should be even better than that.

Y.
*/
