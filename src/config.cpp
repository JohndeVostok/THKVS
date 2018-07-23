#include <cstdio>

int main() {
	freopen("config", "w", stdout);
	for (int i = 0; i < 8; i++) {
		printf("host%d 127.0.0.1\n", i);
	}
}
