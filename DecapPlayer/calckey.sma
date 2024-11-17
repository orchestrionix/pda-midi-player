 #include <console>
myxy(a)
{
	return (a ^ 5878167 + (a >> 5)) / 5
}

//calculate register key
public a(dwCode0, serial)
{
	new d;

	d=0x52415453 ^ dwCode0;
	d=(d*1139) ^ 0x4d5751;
	d=((d << (d & 0xf)) + 211) * 217 + 0x6d7771;
	for(new i = d & 7; i > 0; i--)
		d = (d ^ 3429) * 100 + serial;
	return myxy(d) & 0x4ffffff;
}

public b(x, y, s)
{
	if(x == 0)
		return 0;
	if(a(x, s) == y)
		return 1234;
	return 12;
}

main()
{
	new start;
	new serial;
	start = getvalue(10, '^r')
	serial = getvalue(10, '^r')
	printf("%d-%d,", start, a(start, serial));
}
