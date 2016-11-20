from pwn import *

r = remote('linux2.cs.nctu.edu.tw', 3421)

i = 0
while i < 100:
	print r.recvuntil("%")
	r.sendline("cat test.html |100")
	i = i + 1

i = 100
while i > 0:
	print r.recvuntil("%")
	r.sendline("cat test.html |" + str(i))
	i = i - 1

print r.recvuntil("%")

r.sendline("number")

print r.recvuntil("%")
r.interactive()
