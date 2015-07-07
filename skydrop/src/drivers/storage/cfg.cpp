#include "cfg.h"
#include "../uart.h"

#define CFG_PARSER_NEW_LINE		0
#define CFG_PARSER_SECTION		1
#define CFG_PARSER_LINE_END		2
#define CFG_PARSER_LABEL		3
#define CFG_PARSER_LABEL_END	4
#define CFG_PARSER_VALUE		5

#define CFG_PARSE_BUF	32

int cfg_get_int(FIL * fl, const char * section, const char * label, int def)
{
	char buf[16];

	if (cfg_get_str(fl, section, label, buf))
		return atoi(buf);
	else
		return def;
}

bool cfg_get_str(FIL * fl, const char * section, const char * label, char * value)
{
	if (strlen(section) == 0 || strlen(label) == 0 )
		return false;

	f_lseek(fl, 0);

	uint8_t state = CFG_PARSER_NEW_LINE;
	bool in_section	= false;
	uint8_t label_index = 0;

	uint16_t l;
	uint8_t res;

	do
	{
		uint8_t buf[CFG_PARSE_BUF];
		res = f_read(fl, buf, CFG_PARSE_BUF, &l);

		if (res != FR_OK)
			return false;

		for (uint8_t i = 0; i < l; i++)
		{
			uint8_t c = buf[i];

			switch (state)
			{
				case(CFG_PARSER_NEW_LINE):
					if (c == '[') //section start
					{
		//				if (in_section)
		//					return false;

						state = CFG_PARSER_SECTION;
						label_index = 0;
						in_section = false;
						break;
					}

					if (c == '#')
						state = CFG_PARSER_LINE_END;

					if (c == ' ')
						break;

					if (in_section) //if in section and start is the same as label
						if (c == label[0])
						{
							state = CFG_PARSER_LABEL;
							label_index = 1;
						}
				break;

				case(CFG_PARSER_SECTION):
					if (c == section[label_index]) //if section name is the same
					{
						label_index++;
						if (label_index == strlen(section))
						{
							state = CFG_PARSER_LINE_END;
							in_section = true;
						}
					}
					else //section name is different
						state = CFG_PARSER_LINE_END;
				break;

				case(CFG_PARSER_LINE_END):
					if (c == ']' || c == '\n' || c == '\r')
						state = CFG_PARSER_NEW_LINE;
				break;

				case(CFG_PARSER_LABEL):
					if (c == label[label_index]) //if label name is the same
					{
						label_index++;
						if (label_index == strlen(label))
							state = CFG_PARSER_LABEL_END;
					}
					else //label name is different
						state = CFG_PARSER_LINE_END;
				break;

				case(CFG_PARSER_LABEL_END):
					if (c == ' ' || c == '=') // space and = is ignored
						break;
					else
					{
						value[0] = c;
						state = CFG_PARSER_VALUE;
						label_index = 1;
					}
				break;

				case(CFG_PARSER_VALUE):
					if (c != ' ' && c != '\n' && c != '\r' && c!= '#')
					{
						value[label_index] = c;
						label_index++;
					}
					else
					{
						value[label_index] = 0;
						return true;
					}
			}
		}
	} while (l == CFG_PARSE_BUF);

	return false;
}

bool cfg_have_section(FIL * fl, const char * section)
{
	if (strlen(section) == 0)
		return false;

	f_lseek(fl, 0);

	uint8_t state = CFG_PARSER_NEW_LINE;
	uint8_t label_index = 0;
	uint16_t l;
	uint8_t res;

	bool in_section = false;

	do
	{
		uint8_t buf[CFG_PARSE_BUF];
		res = f_read(fl, buf, CFG_PARSE_BUF, &l);

		if (res != FR_OK)
			return false;

		for (uint8_t i = 0; i < l; i++)
		{
			uint8_t c = buf[i];

			switch (state)
			{
				case(CFG_PARSER_NEW_LINE):
					if (c == '[') //section start
					{
		//				if (in_section)
		//					return false;

						state = CFG_PARSER_SECTION;
						label_index = 0;
						in_section = false;
						break;
					}

					if (c == '#')
						state = CFG_PARSER_LINE_END;

					if (c == ' ')
						break;
				break;

				case(CFG_PARSER_SECTION):
					if (c == section[label_index]) //if section name is the same
					{
						label_index++;
						if (label_index == strlen(section))
						{
							state = CFG_PARSER_LINE_END;
							in_section = true;
						}
					}
					else //section name is different
						state = CFG_PARSER_LINE_END;
				break;

				case(CFG_PARSER_LINE_END):
					if (in_section)
					{
						if (c == ']')
							return true;
						else
							in_section = false;
					}

					if (c == ']' || c == '\n' || c == '\r')
						state = CFG_PARSER_NEW_LINE;
				break;
			}
		}
	} while (l == CFG_PARSE_BUF);

	return false;
}
