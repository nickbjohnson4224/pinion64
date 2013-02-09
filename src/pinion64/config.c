// Copyright (C) 2013 Nick Johnson <nickbjohnson4224 at gmail.com>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "internal.h"

#define CONFIG_INDEX_SIZE 64

static struct config_item {
	const char *section;
	const char *property;
	const char *value;
} config_index[CONFIG_INDEX_SIZE];

static int strcmp(const char *a, const char *b) {
	
	while (*a && *b) {
		if (*(a++) != *(b++)) return 1;
	}

	return 0;
}

void config_parse(char *pconf) {

	// for each line
	size_t index = 0;
	for (size_t i = 0;;) {
		
		switch (pconf[i]) {
		case '#':
			// comment, skip to next line
			for (; pconf[i] && pconf[i] != '\n'; i++);
			if (pconf[i]) i++;
			continue;
		case '\n':
			// empty line, skip
			i++;
			continue;
		case '\0':
			// end of file
			return;
		}

		// skip leading whitespace
		for (; pconf[i] == ' ' || pconf[i] == '\t'; i++);
		if (!pconf[i]) return;

		// parse section
		config_index[index].section = &pconf[i];
		for (; pconf[i] && pconf[i] != ' ' && pconf[i] != '\t' && pconf[i] != '\n'; i++);
		if (!pconf[i]) {
			config_index[index].section = NULL;
			return;
		}
		pconf[i] = '\0';
		i++;

		// skip leading whitespace
		for (; pconf[i] == ' ' || pconf[i] == '\t'; i++);
		if (!pconf[i]) return;

		// parse property name
		config_index[index].property = &pconf[i];
		for (; pconf[i] && pconf[i] != ' ' && pconf[i] != '\t' && pconf[i] != '\n'; i++);
		if (!pconf[i]) {
			config_index[index].section = NULL;
			return;
		}
		pconf[i] = '\0';
		i++;

		// skip leading whitespace
		for (; pconf[i] == ' ' || pconf[i] == '\t'; i++);
		if (!pconf[i]) return;

		// parse value
		config_index[index].value = &pconf[i];
		for (; pconf[i] && pconf[i] != '\n'; i++);
		if (!pconf[i]) {
			config_index[index].section = NULL;
			return;
		}
		pconf[i] = '\0';
		i++;

		index++;
		if (index == CONFIG_INDEX_SIZE) return;
	}
}

const char *config_read(const char *section, const char *property) {
	
	for (int i = 0; i < CONFIG_INDEX_SIZE && config_index[i].section; i++) {
		if (!strcmp(config_index[i].section, section) 
			&& !strcmp(config_index[i].property, property)) {

			return config_index[i].value;
		}
	}

	return NULL;
}

uint64_t config_read_as_ptr(const char *section, const char *property, uint64_t sentinel) {
	const char *value = config_read(section, property);

	uint64_t ptrvalue = 0;

	if (!value) {
		return sentinel;
	}

	for (size_t i = 0; value[i]; i++) {
		ptrvalue <<= 4;

		if (value[i] >= '0' && value[i] <= '9') {
			ptrvalue |= (value[i] - '0');
		}
		else if (value[i] >= 'a' && value[i] <= 'f') {
			ptrvalue |= (value[i] - 'a' + 10);
		}
		else if (value[i] >= 'A' && value[i] <= 'F') {
			ptrvalue |= (value[i] - 'A' + 10);
		}
		else {
			return sentinel;
		}
	}

	return ptrvalue;
}
