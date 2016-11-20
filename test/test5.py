from pwn import *

r = remote('linux2.cs.nctu.edu.tw', 3421)

r.recvuntil("%")
r.sendline("ls |1")

i = 0
while i < 1008:
	print r.recvuntil("%")
	r.sendline("cat |1")
	i = i + 1

print r.recvuntil("%")

r.sendline("cat")

print r.recvuntil("%")
r.interactive()
