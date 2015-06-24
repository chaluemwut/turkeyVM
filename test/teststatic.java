import java.util.ArrayList;



public class teststatic extends Object 
{
	public static void main(String[] args)
	{
		ArrayList<Integer> list = new ArrayList<Integer>();
		System.out.println(list.size());
		list.add(new Integer(1));
		System.out.println(list.size());
		teststatic test[] = new teststatic[10];
		long l[] = new long[10];
		short s[]= new short[20];
		float f[]= new float[20];
		double d[]= new double[20];
		
		l[1] = 8;
		l[2] = 2;
		l[3] = 3;
		l[4] = 4;
		System.out.println(l[2]);
		l[2] = -l[2];
		System.out.println(l[2]);
		
		
		s[2] = 2;
		s[1] = 1;
		s[3] = 3;
		System.out.println(s[3]);
		l[4] = l[3]-l[2];
		s[4] = (short) (s[2]-s[3]);
		System.out.println(l[4]);
		System.out.println(s[4]);
		
		
		f[1]=9;f[2]=10;
		f[3] = f[1]-f[2];
		d[1]=4.5;d[2]=8.4;
		d[3] = d[1]-d[2];
		System.out.println(f[3]);
		System.out.println(d[3]);
		
		l[3] = l[1]*l[2]/l[3];
		f[3] = f[1]*f[2]/f[1];
		d[3] = d[1]+d[2]/d[3];
		System.out.println(l[3]);
		System.out.println(f[3]);
		System.out.println(d[3]);
		f[3] = -f[3];
		d[3] = -d[3];
		int i1=5;
		int i2 = 6;
		int i3 = 98;
		l[3] = l[1]^l[2];
		l[3] = i3+l[2];
		d[3] = l[3];
		char c = (char) i3;
		i3 = (int) d[2];
		f[3] =(float) d[2];
		l[3] = (long) d[2];
		i3 =  (int) f[3];
		l[3] = (long) f[3];
		d[3] = f[3];
		f[3] = l[3];
		f[3] = i3;
		if (l[3]<l[2])
		System.out.println(i3);
		System.out.println(l[3]);
		System.out.println(d[3]);
		System.out.println(c);
		
		
		
//		char c[] = new char[10];
//		Testsub[]t = new Testsub[10]; 
//		t[1] = new Testsub(1,50);
//		t[3] = new Testsub(50,1);
//		int i =t[1].sub(t[3].a, t[3].b);
//		int j = t[3].sub(t[1].a, t[1].b);
//		Integer i1 = new Integer(1);
//		System.out.println(i1);	
//		
////		int i = 2;
////		synchronized(i1)
////		{
////			i<<=2;
////		}
////		System.out.println(i);
//		
//		if (i1 instanceof Integer)
//			System.out.println(i1.getClass().getName());
//		Testsub a = new Testsub(1,50);
//		if (a instanceof Object)
//			System.out.println(a.getClass().getName());
//
//		System.out.println(i);
//		System.out.println(j);
	}
	
}
