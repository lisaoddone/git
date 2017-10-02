#include "cache.h"
#include "packfile.h"

struct count {
	int total;
	int select_mod;
};

int count_loose(const struct object_id *oid,
		const char *path,
		void *data)
{
	((struct count*)data)->total++;
	return 0;
}

int count_packed(const struct object_id *oid,
		 struct packed_git *pack,
		 uint32_t pos,
		 void* data)
{
	((struct count*)data)->total++;
	return 0;
}

void output(const struct object_id *oid,
	    struct count *c)
{
	if (!(c->total % c->select_mod))
		printf("%s\n", oid_to_hex(oid));
	c->total--;
}

int output_loose(const struct object_id *oid,
		 const char *path,
		 void *data)
{
	output(oid, (struct count*)data);
	return 0;
}

int output_packed(const struct object_id *oid,
		  struct packed_git *pack,
		  uint32_t pos,
		  void* data)
{
	output(oid, (struct count*)data);
	return 0;
}

int cmd_main(int ac, const char **av)
{
	uint32_t hash_delt = 0xFDB97531;
	uint32_t hash_base = 0x01020304;
	int i, n = -1;
	struct count c;
	const int u_per_oid = sizeof(struct object_id) / sizeof(uint32_t);

	c.total = 0;
	if (ac > 1)
		n = atoi(av[ac - 1]);

	if (ac > 2 && !strcmp(av[1], "--missing")) {
		while (c.total++ < n) {
			for (i = 0; i < u_per_oid; i++) {
				printf("%08x", hash_base);
				hash_base += hash_delt;
			}
			printf("\n");
		}
	} else {
		setup_git_directory();

		if (n > 0) {
			for_each_loose_object(count_loose, &c, 0);
			for_each_packed_object(count_packed, &c, 0);
			c.select_mod = 1 + c.total / n;
		} else {
			c.select_mod = 1;
		}

		for_each_loose_object(output_loose, &c, 0);
		for_each_packed_object(output_packed, &c, 0);
	}

	exit(0);
}
