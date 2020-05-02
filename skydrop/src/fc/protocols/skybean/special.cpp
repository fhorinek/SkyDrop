#include "special.h"

#include "../../fc.h"

uint8_t login_level = 0;

uint8_t skybean_special_login()
{
	uint8_t len = skybean_stream.Read();
	uint8_t ret = SB_SUCCESS;

	if (len == 0 || len > LOGIN_PASSWORD_LEN)
		return SB_FAIL;

	for (uint8_t i = 0; i < len; i++)
		if (skybean_stream.Read() != config.connectivity.password[i])
			ret = SB_FAIL;

	if (len < LOGIN_PASSWORD_LEN)
		if (config.connectivity.password[len] != 0)
			ret = SB_FAIL;

	return ret;
}

uint8_t skybean_special_change()
{
	uint8_t len = skybean_stream.Read();
	uint8_t tmp_passwd[LOGIN_PASSWORD_LEN];
	uint8_t ret = SB_SUCCESS;

	if (len < 4 || len > LOGIN_PASSWORD_LEN || login_level == 0)
		return SB_FAIL;

	

	for (uint8_t i = 0; i < len; i++)
		tmp_passwd[i] = skybean_stream.Read();

	if (len < LOGIN_PASSWORD_LEN)
		tmp_passwd[len] = SB_FAIL;

	memcpy((void *)config.connectivity.password, tmp_passwd, LOGIN_PASSWORD_LEN);

	
	ee_update_block((void *)config.connectivity.password, config_ee.connectivity.password, LOGIN_PASSWORD_LEN);

	return ret;
}

void skybean_special(uint8_t op)
{
	uint8_t ret;

	switch(op)
	{
		case(0x00): //login status
			skybean_startpacket(1, SKYBEAN_SPECIAL, op);
			skybean_stream.Write(login_level); //len
		break;

		case(0x01): //login request
			login_level = skybean_special_login();

			skybean_startpacket(1, SKYBEAN_SPECIAL, op);
			skybean_stream.Write(login_level);
		break;

		case(0x02): //logout request
			login_level = 0;
			skybean_startpacket(0, SKYBEAN_SPECIAL, op);
		break;

		case(0x03): //change password
			ret = skybean_special_change();
			skybean_startpacket(1, SKYBEAN_SPECIAL, op);
			skybean_stream.Write(ret);
		break;

		default:
			skybean_startpacket(0, SKYBEAN_SPECIAL, SB_ERROR);
	}
}
