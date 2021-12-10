/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_pars_serv.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmoreau <vmoreau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 15:56:18 by vmoreau           #+#    #+#             */
/*   Updated: 2021/12/10 16:24:00 by vmoreau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../incs/webserv.hpp"
#include "conf/confpars.hpp"

int main()
{
	confpars conf;

	try
	{
		conf.open_fc(CONF_DEFAULT_PATH);
		conf.pars_fc(conf.get_fd());
		conf.close_fc();
	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "ERROR: " << NC << e.what() << '\n';
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}