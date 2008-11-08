/** \file instmanager.hpp
 *
 * Created: JohnE, 2008-11-01
 */
#ifndef INSTMANAGER_HPP_INC
#define INSTMANAGER_HPP_INC


#include <string>


class InstManager
{
public:
	static bool InstExistsAt(const std::string& path);

	static bool Load(const char* inst_path);

private:
	static std::string sm_inst_path;
};


#endif // INSTMANAGER_HPP_INC
