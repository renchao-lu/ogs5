class NR
{
private:

public:
	NR(void);
	~NR(void);

	static double dfridr (double func(double),        const double x);
	static double dfridrX(double func(double,double), const double x, const double y);
	static double dfridrY(double func(double,double), const double x, const double y);

};

