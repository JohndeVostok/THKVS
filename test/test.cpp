#include <thread>

using namespace std;

void fuck()
{
		Data::getInstance()->run();
}

int main()
{
	Thread t(fuck, this);
	t.join();

}
