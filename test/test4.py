from pwn import *

r = remote('linux2.cs.nctu.edu.tw', 3421)

r.recvuntil("%")
r.sendline("removetag0 test.html !999")

i = 0
while i < 998:
	print r.recvuntil("%")
	r.sendline("noop")
	i = i + 1

print r.recvuntil("%")

r.sendline("number | cat")

print r.recvuntil("%")
r.interactive()
