#ifndef SSG_COLOR4_H_
#define SSG_COLOR4_H_

struct ssg_color4f
{
	float r, g, b, a;

	ssg_color4f()
		: r(1), g(1), b(1), a(1) {}
	ssg_color4f(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a) {}
};

#endif // SSG_COLOR4_H_