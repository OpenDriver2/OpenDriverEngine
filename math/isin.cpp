// Based on isin_S4 implementation from coranac:
// http://www.coranac.com/2009/07/sines/

static constexpr int qN = 10;
static constexpr int qA = 12;
static constexpr int B = 19900;
static constexpr int C = 3516;

/// @param x	angle (with 2^15 units/circle)
/// @return     Sine value (Q12)
int isin(int x)
{
	int c, x2, y;

	c = x << (30 - qN);				// Semi-circle info into carry.
	x -= 1 << qN;					// sine -> cosine calc

	x = x << (31 - qN);				// Mask with PI
	x = x >> (31 - qN);				// Note: SIGNED shift! (to qN)

	x = x * x >> (2 * qN - 14);		// x=x^2 To Q14

	y = B - (x * C >> 14);			// B - x^2*C
	y = (1 << qA) - (x * y >> 16);	// A - x^2*(B-x^2*C)

	return c >= 0 ? y : -y;
}

int icos(int x)
{
	return isin(x + 1024);
}