#ifndef SSG_TIMER_H_
#define SSG_TIMER_H_

class ssg_timer_impl;

class ssg_timer
{
public:
	ssg_timer();
	~ssg_timer();

	void start();
	void reset();
	float elapsedTime();

private:
	ssg_timer_impl* impl;
};

#endif // SSG_TIMER_H_