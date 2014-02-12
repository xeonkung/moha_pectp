#ifndef move_h_
#define move_h_



class move
{
public:
	
	int x,y;
	int Hcv, Scv;
	
	move(int i=-1, int j = -1):	x(i),y(j),Hcv(99999),Scv(99999){}

	void reset(int i = -1,int j = -1, int h = 99999, int s = 99999)
	{
		x = i;
		y = j;
		Hcv = h;
		Scv = s;
	}

	bool empty()
	{
		if(x<0)
			return true;
		return false;
	}
};

#endif