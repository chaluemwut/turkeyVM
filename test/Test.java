import java.util.HashMap;


public class Test extends Object
{
	
	public static int userAdd(int a, int b)
	{
		return a+b;
	}
	public static void main(String[] args)
	{
		System.out.println("\ntest case1: hello, world");
		HashMap map = new HashMap();
		map.put("qc1iu", "hello,world");
		System.out.println(map.get("qc1iu"));
		
		System.out.println("\ntest case2: sum");
		System.out.print("1+2+...+100=");
		int sum=0;
		for (int i=1; i<=100; i++)
		{
			sum+=i;
		}
		sum+=userAdd(0, 0);
		System.out.println(String.valueOf(sum));
		
		System.out.println("\ntest case3: args");
		for (int i=0; i<args.length; i++)
			System.out.println(args[i]);
		
	}

}
