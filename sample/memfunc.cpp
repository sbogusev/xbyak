#include <stdio.h>
#include <stdlib.h>
#include <xbyak/xbyak.h>

struct A {
	int x_;
	int y_;
	A() : x_(3), y_(5) {}
	int func(int a, int b, int c, int d, int e) const { return x_ + y_ + a + b + c + d + e; }
};

struct Code : public Xbyak::CodeGenerator {
	Code()
	{
		using namespace Xbyak;

		int RET_ADJ = 0;
#ifdef XBYAK32
	#ifdef _WIN32
		const int PARA_ADJ = 0;
		RET_ADJ = 5 * 4;
	#else
		const int PARA_ADJ = 4;
		mov(ecx, ptr [esp + 4]);
	#endif
#endif
		const struct {
			const Reg32e& self;
			const Operand& a;
			const Operand& b;
			const Operand& c;
			const Operand& d;
			const Operand& e;
		} para = {
#if defined(XBYAK64_WIN)
			rcx,
			edx,
			r8d,
			r9d,
			ptr [rsp + 8 * 5],
			ptr [rsp + 8 * 6],
#elif defined(XBYAK64_GCC)
			rdi,
			esi,
			edx,
			ecx,
			r8d,
			r9d,
#else
			ecx,
			ptr [esp + 4 + PARA_ADJ],
			ptr [esp + 8 + PARA_ADJ],
			ptr [esp + 12 + PARA_ADJ],
			ptr [esp + 16 + PARA_ADJ],
			ptr [esp + 20 + PARA_ADJ],
#endif
		};
		mov(eax, ptr [para.self]);
		add(eax, ptr [para.self + 4]);
		add(eax, para.a);
		add(eax, para.b);
		add(eax, para.c);
		add(eax, para.d);
		add(eax, para.e);
		ret(RET_ADJ);
	}
};

int main()
{
#ifdef XBYAK64
	printf("64bit");
#else
	printf("32bit");
#endif
#ifdef _WIN32
	puts(" win");
#else
	puts(" linux");
#endif
	try {
		Code code;
		int (A::*p)(int, int, int, int, int) const = 0;
#if defined(XBYAK32) && !defined(_WIN32)
		// avoid breaking strict-aliasing rules for 32bit gcc
		union {
			int (A::*p)(int, int, int, int, int) const;
			const Xbyak::uint8 *code;
		} u;
		u.code = code.getCode();
		p = u.p;
#else
		*(void**)&p = code.getCode<void*>();
#endif
		for (int i = 0; i < 10; i++) {
			A a;
			int t1, t2, t3, t4, t5, x, y;
			a.x_ = rand(); a.y_ = rand();
			t1 = rand(); t2 = rand(); t3 = rand();
			t4 = rand(); t5 = rand();
			x = a.func(t1, t2, t3, t4, t5);
			y = (a.*p)(t1, t2, t3, t4, t5);
			printf("%c %d, %d\n", x == y ? 'o' : 'x', x, y);
		}
	} catch (Xbyak::Error& e) {
		printf("err=%s\n", Xbyak::ConvertErrorToString(e));
		return 1;
	}
}

