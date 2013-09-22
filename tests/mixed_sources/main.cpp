extern "C"{
	int c_test();
}

int cpp_test();

int main()
{
	return c_test() + cpp_test();
}
