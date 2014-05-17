<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | functions.inc                                                            |
// |                                                                          |
// | This file implements the necessary glFusion Plugin API functions and     |
// | includes some common GUS functions                                       |
// +--------------------------------------------------------------------------+
// | $Id::                                                                   $|
// +--------------------------------------------------------------------------+
// | Copyright (C) 2009-2013 by the following authors:                        |
// |                                                                          |
// | Mark R. Evans          mark AT glfusion DOT org                          |
// |                                                                          |
// | Based on the GUS Plugin for Geeklog CMS                                  |
// | Copyright (C) 2002, 2003, 2005 by the following authors:                 |
// |                                                                          |
// | Authors: Andy Maloney      - asmaloney@users.sf.net                      |
// |          Tom Willett       - twillett@users.sourceforge.net              |
// |          John Hughes       - jlhughes@users.sf.net                       |
// +--------------------------------------------------------------------------+
// |                                                                          |
// | This program is free software; you can redistribute it and/or            |
// | modify it under the terms of the GNU General Public License              |
// | as published by the Free Software Foundation; either version 2           |
// | of the License, or (at your option) any later version.                   |
// |                                                                          |
// | This program is distributed in the hope that it will be useful,          |
// | but WITHOUT ANY WARRANTY; without even the implied warranty of           |
// | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            |
// | GNU General Public License for more details.                             |
// |                                                                          |
// | You should have received a copy of the GNU General Public License        |
// | along with this program; if not, write to the Free Software Foundation,  |
// | Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.          |
// |                                                                          |
// +--------------------------------------------------------------------------+

if (!defined ('GVERSION')) {
    die ('This file can not be used on its own.');
}

$langfile = $_CONF['path'] . 'plugins/gus/language/' . $_CONF['language'] . '.php';

if ( file_exists( $langfile ) ) {
    include_once $langfile;
} else {
    require_once $_CONF['path'].'plugins/gus/language/english.php';
}

require_once $_CONF['path'].'plugins/gus/gus.php';
require_once $_CONF['path'].'plugins/gus/config.php';

///////////////////////////////////////////////////////////////////
///  Set up config variables and call to add stats to database  //
/////////////////////////////////////////////////////////////////

GUS();

function GUS()
{
	global $_GUS_VARS, $_GUS_CONF, $_USER, $_TABLES, $_CONF;

	$_GUS_VARS = array();

	$rec = DB_query( "SELECT * FROM {$_TABLES['gus_vars']}", 1 );

	if ( DB_numRows( $rec ) > 0 )
	{
		// Get the GUS vars from the DB
		while ( $row = DB_fetchArray( $rec ) )
		{
			$_GUS_VARS[$row['name']] = $row['value'];
		}

		// store the MySQL version number in $_GUS_VARS['sql_version']
/*
		$version = mysql_get_server_info();

		$_GUS_VARS['sql_version'] = array();

		if ( eregi( "^([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})", $version, $match ) )
		{
			$_GUS_VARS['sql_version']['major'] = $match[1];
			$_GUS_VARS['sql_version']['minor'] = $match[2];
			$_GUS_VARS['sql_version']['patch'] = $match[3];
		}
*/
		// save our parsed url for path info later on
		$_GUS_VARS['parsed_site_url'] = parse_url( $_CONF['site_url'] );

		if ( $_GUS_VARS['capture'] == '1' )
		{
			// First check the IP...
			$_GUS_VARS['remote_ip'] = GetIP( $_SERVER['REMOTE_ADDR'] );

			$rec = DB_query( "SELECT COUNT(*) AS ignored
								FROM {$_TABLES['gus_ignore_ip']}
								WHERE '{$_GUS_VARS['remote_ip']}' LIKE ip
								LIMIT 1", 1 );

			$row = DB_fetchArray( $rec, false );

			if ( $row['ignored'] == '1' )
				return;

			// Next check the hostname...
			$host_name = ip_to_hostname( $_GUS_VARS['remote_ip'] );

			$rec = DB_query( "SELECT COUNT(*) AS ignored
								FROM {$_TABLES['gus_ignore_host']}
								WHERE '{$host_name}' LIKE host
								LIMIT 1", 1 );

			$row = DB_fetchArray( $rec, false );

			if ( $row['ignored'] == '1' )
				return;

			// Next check the page...

			// remove extra path from the page name if necessary
			$page = $_SERVER['SCRIPT_NAME'];
			if ( isset($_GUS_VARS['parsed_site_url']['path']) ) {
    			$len = strlen( $_GUS_VARS['parsed_site_url']['path'] );
    		} else {
    		    $len = 0;
    		}

			if ( isset($_GUS_VARS['parsed_site_url']['path']) ) {
    			if ( strncmp( $page, $_GUS_VARS['parsed_site_url']['path'], $len ) == 0 )
    				$page = substr( $page, $len + 1 );
 			}

			$rec = DB_query( "SELECT COUNT(*) AS ignored
								FROM {$_TABLES['gus_ignore_page']}
								WHERE '{$page}' LIKE page
								LIMIT 1", 1 );

			$row = DB_fetchArray( $rec, false );

			if ( $row['ignored'] == '1' )
				return;

			// check the user...
			$check_user = true;

			if ( !isset( $_USER['uid'] ) || ($_USER['uid'] < 2) )
			{
				$user_id = 1;
				$user_name = 'Anonymous';

				$check_user = $_GUS_CONF['allow_ignore_anonymous'];
			}
			else
			{
				$user_id = $_USER['uid'];
				$user_name = $_USER['username'];
			}

			if ( $check_user )
			{
				$rec = DB_query( "SELECT COUNT(*) AS ignored
									FROM {$_TABLES['gus_ignore_user']}
									WHERE '{$user_name}' LIKE username
									LIMIT 1", 1 );

				$row = DB_fetchArray( $rec, false );

				if ( $row['ignored'] == '1' )
					return;
			}

			// check user agent
			$user_agent = DB_escapeString( substr( $_SERVER['HTTP_USER_AGENT'], 0, 128 ) );

			$rec = DB_query( "SELECT COUNT(*) AS ignored
								FROM {$_TABLES['gus_ignore_ua']}
								WHERE '{$user_agent}' LIKE ua
								LIMIT 1", 1 );

			$row = DB_fetchArray( $rec, false );

			if ( $row['ignored'] == '1' )
				return;

			// check the referrer
			if ( isset( $_SERVER['HTTP_REFERER'] ) && ( $_SERVER['HTTP_REFERER'] != '' ) )
			{
				$referrer = $_SERVER['HTTP_REFERER'];

				$rec = DB_query( "SELECT COUNT(*) AS ignored
									FROM {$_TABLES['gus_ignore_referrer']}
									WHERE '{$referrer}' LIKE referrer
									LIMIT 1", 1 );

				$row = DB_fetchArray( $rec, false );

				if ( $row['ignored'] == '1' )
					return;
			}

			// Finally, add stats
			GUS_addStats( $page, $user_id, $user_name, $host_name, $user_agent );
		} else {
		    $_GUS_VARS['remote_ip'] = '';
		}
	}
}

// adapted from code found here: http://www.php.net/getenv
function GetIP( $ip )
{
// commented out because it seems to give misleading results sometimes
/*
	if ( $_SERVER["HTTP_CLIENT_IP"] && strcasecmp($_SERVER["HTTP_CLIENT_IP"], "unknown") )
		$ip = $_SERVER["HTTP_CLIENT_IP"];
	else if ( $_SERVER["HTTP_X_FORWARDED_FOR"] && strcasecmp($_SERVER["HTTP_X_FORWARDED_FOR"], "unknown") )
		$ip = $_SERVER["HTTP_X_FORWARDED_FOR"];
*/
	// IF more than one IP, grab the first
	if ( strstr( $ip, ', ' ) )
	{
		$ips = explode(', ', $ip);
		$ip = $ips[0];
	}

	return( $ip );
}

// adapted from code found here: http://www.php.net/gethostbyaddr
function ip_to_hostname( $ip )
{
	global $_GUS_CONF;


	switch( $_GUS_CONF['host_lookup'] ) {
		case 'host':
			$timeout = $_GUS_CONF['host_lookup_timeout'];

			$output = `host -W $timeout $ip`;

			if ( ereg( '.*pointer ([A-Za-z0-9.-]+)\..*', $output, $regs ) )
				return $regs[1];

			break;

		case 'nslookup':
			$timeout = $_GUS_CONF['host_lookup_timeout'];

			$res = `nslookup -timeout=$timeout $ip`;

			if ( preg_match( '/\nName:(.*)\n/', $res, $out ) )
				return trim( $out[1] );

			break;

		case 'gethostbyaddr':
			return( @gethostbyaddr( $ip ) );
			break;

        case 'none':
            return $ip;
            break;

		default:
			COM_errorLog( "Invalid setting for 'host_lookup' in config.php [{$_GUS_CONF['host_lookup']}]" );
			break;
	}

	return $ip;
}

// +---------------------------------------------------------------------------+
// | glFusion Plugin API Implementations                                       |
// +---------------------------------------------------------------------------+

/**
 * Returns the current plugin code version
 *
 * @return string    plugin version
*/
function plugin_chkVersion_gus()
{
    global $_GUS_CONF;

	return $_GUS_CONF['pi_version'];
}

// Add the CSS file if we are browsing a GUS page
function	plugin_getheadercode_gus()
{
	global $_CONF;

    $this_dir = basename( dirname( $_SERVER['SCRIPT_NAME'] ) );

	if ( $this_dir != 'gus' )
		return '';

	$header_text = '';

	// check for CSS file specific to this theme
	$css_file = 'css/' . $_CONF['theme'] . '.css';
	$css_path = $_CONF['path_html'] . 'gus/' . $css_file;

	// IF we don't find one that's readable THEN use the default
	if ( !is_readable( $css_path ) )
	{
		$css_file = 'css/default.css';
		$css_path = $_CONF['path_html'] . 'gus/' . $css_file;

		// IF we still don't have anything THEN return
		if ( !is_readable( $css_path ) )
			return '';
	}

	$stylesheet = $_CONF['site_url'] . '/gus/' . $css_file;

	$header_text .= '<link rel="stylesheet" type="text/css" href="' . $stylesheet . '"/>';

	if ( strpos( $_SERVER['SCRIPT_NAME'], 'admin/plugins/gus/index.php' ) )
		$header_text .= '<script type="text/javascript" src="./Tab.js"></script>';

	return $header_text;
}

/**
* Returns the items for this plugin that should appear on the main menu
*
* NOTE: this MUST return the url/value pairs in the following format
* $<arrayname>[<label>] = <url>
*
*/
function plugin_getmenuitems_gus()
{
    global $_CONF, $LANG_GUS00, $_GUS_anon_access, $_GUS_enable_main_menu_GUS, $_GUS_enable_main_menu_privacy_policy;

    $menuitems = array();

    if ( $_GUS_enable_main_menu_GUS && ($_GUS_anon_access || SEC_hasRights( 'gus.view' )) )
    {
        $tmp = $LANG_GUS00['main_menu_title'];
        $menuitems["$tmp"] = $_CONF['site_url'] . '/gus/index.php';
    }

    if ( $_GUS_enable_main_menu_privacy_policy == 1 )
    {
        $tmp = $LANG_GUS00['priv_pol'];
        $menuitems["$tmp"] = $_CONF['site_url'] . '/gus/privpol.php';
    }

    return $menuitems;
}

/**
* Shows the statistics for the stats plugin on stats.php.  If
* $showsitestats is 1 then we are to only print the overall stats in the 'site
* statistics' box otherwise we show the detailed stats for the stats
*
* @showsitestate        int         Flag to let us know which stats to get
*/
function plugin_statssummary_gus()
{
	global $_TABLES, $LANG_GUS_builtin_stats, $_GUS_stats;

    if ( $_GUS_stats == 1)
	{
        $rec = DB_query( "SELECT COUNT( DISTINCT ip ) AS visitors FROM {$_TABLES['gus_userstats']}" );
		$row = DB_fetchArray( $rec, false );
        $retval[0][] = $LANG_GUS_builtin_stats['unique_visitors'];
        $retval[0][] = $row['visitors'];
        $total_stats = DB_count( $_TABLES['users'] );
        $retval[1][] = $LANG_GUS_builtin_stats['users'];
        $retval[1][] = $total_stats;
        return $retval;
    }
}

/**
* This will put an option for stats in the command and control block on
* moderation.php
*
*/
function plugin_cclabel_gus()
{
	global $LANG_GUS00, $_CONF, $_GUS_IMG_name;

	$retval = array();

	if ( SEC_hasRights('gus.admin') )
	{
		$retval = array( $LANG_GUS00['GUS_title'],
			$_CONF['site_admin_url'] . '/plugins/gus/index.php',
			$_CONF['site_url'] . '/gus/images/' . $_GUS_IMG_name );
	}

	return $retval;
}

/**
* returns the administrative option for this plugin
*
*/
function plugin_getadminoption_gus()
{
    global $LANG_GUS_blocks, $_CONF;

    if ( SEC_hasRights( 'gus.admin' ) )
    {
        return array( $LANG_GUS_blocks['admin_menu_title'], $_CONF['site_admin_url'] . '/plugins/gus/index.php', 0 );
    }
}

/**
* returns the user option for this plugin
*
*/
function plugin_getuseroption_gus()
{
    global $LANG_GUS_blocks, $_CONF, $_GUS_user;

    if ( SEC_hasRights( 'gus.view' ) && ($_GUS_user == 1) )
    {
    	$title = $LANG_GUS_blocks['user_menu_title'];

    	// Micro-hack:
    	//	We're only supposed to return one thing, but we close the anchor and open another
    	//	in order to return two...
    	$title .= '</a> <a href="' . $_CONF['site_url'] . '/gus/day.php' . '">[' . $LANG_GUS_blocks['today'] . ']';

        return array( $title, $_CONF['site_url'] . '/gus/index.php?page=latest', 0 );
    }
}

// upgrade to this version of GUS
function plugin_upgrade_gus()
{
	global $_TABLES, $_GUS_CONF;

	$res = DB_query( "SELECT pi_version FROM {$_TABLES['plugins']} WHERE pi_name = 'gus'" );
	$row = DB_fetchArray( $res, false );

	COM_errorLog( "Updating GUS plugin from v" . $row['pi_version'] . ' to v' . plugin_chkVersion_gus(), 1 );

	return GUS_doUpgrades( $row['pi_version'] );
}

function GUS_doUpgrades( $current_gus_version )
{
	global $_TABLES, $_CONF, $_DB, $_GUS_CONF;

	$_DB->setDisplayError( true );

	$done = false;

	while ( $done == false )
	{
		switch ( $current_gus_version )
		{
			case '1.0.0':
			case '1.1.0':
			case '1.2.0':
				$current_gus_version = '1.3.0';

			case '1.3.0':
			{
				require_once( $_CONF['path'] . 'plugins/gus/sql/updates/gus_1.3.0_to_1.3.1.php' );

				for ( $i = 1; $i <= count( $_SQL ); $i++ )
				{
					DB_query( current( $_SQL ) );
					next( $_SQL );
				}

				$current_gus_version = '1.3.1';
			}

			case '1.3.1':
			case '1.3.2':
				$current_gus_version = '1.3.3';

			case '1.3.3':
			{
				$_SQL = array();

				require_once( $_CONF['path'] . 'plugins/gus/sql/updates/gus_1.3.3_to_1.4.0.php' );

				for ( $i = 1; $i <= count( $_SQL ); $i++ )
				{
					DB_query( current( $_SQL ) );
					next( $_SQL );
				}

				GUS_UPD_fix_pages();

				$current_gus_version = '1.4.0';
			}

			case '1.4.0':
			{
				$_SQL = array();

				require_once( $_CONF['path'] . 'plugins/gus/sql/updates/gus_1.4.0_to_1.5.0.php' );

				for ( $i = 1; $i <= count( $_SQL ); $i++ )
				{
					DB_query( current( $_SQL ) );
					next( $_SQL );
				}

				$current_gus_version = '1.5.0';
			}

			case '1.5.0':
			case '1.6.0':
			case '1.7.0':
			    require_once $_CONF['path'] . 'plugins/gus/sql/updates/gus_1.5.0_to_2.0.0.php';
				for ( $i = 1; $i <= count( $_SQL ); $i++ )
				{
					DB_query( current( $_SQL ),1 );
					next( $_SQL );
				}
				$current_gus_version = '2.0.0.fusion';
            case '2.0.0.fusion' :
            case '2.1.0' :
            case '2.1.1' :
            case '2.1.3' :
            case '2.1.4' :
            case '2.1.5' :
            case '2.1.6' :
            case '2.1.7' :
                DB_query("UPDATE {$_TABLES['groups']} SET grp_gl_core=2 WHERE grp_name='Gus Admin'",1);
                // no updates required...
				$current_gus_version = $_GUS_CONF['pi_version'];
			default:
				$current_gus_version = $_GUS_CONF['pi_version'];
				$done = true;
				break;
		}
	}
	DB_query( "UPDATE {$_TABLES['plugins']} SET pi_version = '{$_GUS_CONF['pi_version']}' WHERE pi_name = 'gus'" );
	DB_query( "UPDATE {$_TABLES['plugins']} SET pi_gl_version = '{$_GUS_CONF['gl_version']}',pi_homepage='http://www.glfusion.org' WHERE pi_name = 'gus'" );

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//  STATS Functions                                                             //
//////////////////////////////////////////////////////////////////////////////////

// Check for the 'stats' plugin, which may be installed under a different name
//	return true/false
function GUS_checkStatsInstall()
{
	global	$_CONF, $_TABLES, $_DB_table_prefix, $_ST_plugin_name;

	if ( empty( $_ST_plugin_name ) )
	{
		$_ST_plugin_name = DB_getItem( $_TABLES['vars'], 'value', "name = 'stats_name'" );

		if ( empty( $_ST_plugin_name ) )
		   return false;

		// include the config file so we get the $_TABLE vars
		require_once( $_CONF['path'] . 'plugins/' . $_ST_plugin_name . '/config.php' );

		// now that we have the table names, check that the old userstats table actually exists
		$res = DB_query( "SELECT uid FROM {$_TABLES['userstats']} LIMIT 1", 1 );

		return( !DB_error() );
	}

	return true;
}

// Because the referrers are stored encoded, we need our URLs to look similar for SQL comparison
//	- make it look like the referrers by encoding it
//	- escape '%' for the LIKE clause
function GUS_getEncodedURL( $url )
{
	$encoded_site_url = urlencode( DB_escapeString( $url ) );
	$encoded_site_url = str_replace( '%', '\%', $encoded_site_url );

	return $encoded_site_url;
}

/*
* Return an array with Browser type and Version
*/
function GUS_getBrowser( $UA )
{
    $browser = array();

    if ( @eregi( "opera[ /]([0-9]{1,2}.[0-9]{1,3}){0,1}", $UA, $match ) )
    {
        $browser['type'] = "Opera";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "konqueror/([\.\+[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = "Konqueror";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "lynx/([\.\+[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = "Lynx";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "links \(([0-9]{1,2}.[0-9]{1,3})", $UA, $match ) )
    {
        $browser['type'] = "Links";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "(Safari|Camino|Firefox|Firebird)/([\.\+[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = $match[1];
        $browser['ver'] = $match[2];
    }
    elseif ( eregi( "msie ([\.\+[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = "MSIE";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "^Mozilla/5.0 \(compatible; Yahoo\! Slurp;", $UA ) )
    {
        $browser['type'] = "Yahoo! Slurp";
        $browser['ver'] = '1.0';
    }
    elseif ( eregi( "netscape6/(6.[0-9]{1,3})", $UA, $match ) )
    {
        $browser['type'] = "Netscape";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "netscape/(7.[0-9]{1,3})", $UA, $match ) )
    {
        $browser['type'] = "Netscape";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "^mozilla/5.+ rv:([^\)]+)\)", $UA, $match ) )
    {
        $browser['type'] = "Mozilla";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "^mozilla/([\.\+[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = "Netscape";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "^Jigsaw/([^ ]+)", $UA, $match ) )
    {
        $browser['type'] = "W3C CSS Validator";
        $browser['ver'] = $match[1];
    }
    elseif ( eregi( "w3m", $UA ) )
    {
        $browser['type'] = "w3m";
        $browser['ver'] = "Unknown";
    }
    elseif ( eregi( "^([^/]+)/([\.[:digit:]]+)", $UA, $match ) )
    {
        $browser['type'] = strtr( $match[1], '_', ' ' );
        $browser['ver'] = $match[2];
    }
    else
    {
        $browser['type'] = "Unknown";
        $browser['ver'] = "Unknown";
    }

    return $browser;
}

/*
* Return Computer Type
*/
function GUS_getComputerType( $UA )
{
	if ( eregi( "linux", $UA ) )
	{
    	$return = "Linux";
	}
	elseif ( eregi( "win32", $UA ) )
	{
    	$return = "Windows";
	}
	elseif ( ( eregi( "(win)([0-9]{2})", $UA, $match ) ) || ( eregi( "(windows) ([0-9]{2})", $UA, $match ) ) )
	{
    	$return = "Windows $match[2]";
	}
	elseif ( eregi( "win ?9x", $UA ) )
	{
    	$return = "Windows 9x";
	}
	elseif ( eregi( "Windows XP", $UA ) )
	{
    	$return = "Windows XP";
	}
	elseif ( eregi( "(winnt)([0-9]{1,2}.[0-9]{1,2}){0,1}", $UA, $match ) )
	{
		if ($match[2] == '5.1') $return = "Windows XP";
		elseif ($match[2] == '5.0') $return = "Windows 2000";
		elseif ($match[2] == '5.2') $return = "Windows 2003";
		elseif ($match[2] == '6.0') $return = "Windows Vista";
		else $return = "Windows NT $match[2]";
	}
	elseif ( eregi( "(windows nt) {0,1}([0-9]{1,2}.[0-9]{1,2}){0,1}", $UA, $match ) )
	{
		if ($match[2] == '5.1') $return = "Windows XP";
		elseif ($match[2] == '5.0') $return = "Windows 2000";
		elseif ($match[2] == '5.2') $return = "Windows 2003";
		elseif ($match[2] == '6.0') $return = "Windows Vista";
		else $return = "Windows NT $match[2]";
	}
	elseif ( eregi( "mac os x", $UA ) )
	{
    	$return = "Mac OS X";
	}
	elseif ( eregi( "mac", $UA ) )
	{
    	$return = "MacOS";
	}
	elseif ( eregi( "darwin", $UA ) )
	{
    	$return = "Darwin";
	}
	elseif ( eregi( "(sunos) ([0-9]{1,2}.[0-9]{1,2}){0,1}", $UA, $match ) )
	{
    	$return = "SunOS $match[2]";
 	}
 	elseif ( eregi( "(beos) r([0-9]{1,2}.[0-9]{1,2}){0,1}", $UA, $match ) )
 	{
    	$return = "BeOS $match[2]";
    }
    elseif ( eregi( "freebsd", $UA ) )
    {
        $return = "FreeBSD";
    }
    elseif ( eregi( "openbsd", $UA ) )
    {
        $return = "OpenBSD";
    }
    elseif ( eregi( "irix", $UA ) )
    {
        $return = "IRIX";
    }
    elseif ( eregi( "os/2", $UA ) )
    {
        $return = "OS/2";
    }
    elseif ( eregi( "plan9", $UA ) )
    {
        $return = "Plan9";
    }
    elseif ( eregi( "unix", $UA ) || eregi( "hp-ux", $UA ) )
    {
        $return = "Unix";
    }
    elseif ( eregi( "osf", $UA ) )
    {
        $return = "OSF";
    }
 	elseif ( eregi( "Symbian", $UA ) )
 	{
    	$return = "SymbianOS";
    }
 	elseif ( eregi( "WebTV", $UA ) )
 	{
    	$return = "WebTV";
    }
    elseif ( eregi( "(bot|Slurp|larbin|Jeeves|Spider)", $UA ) )
    {
        $return = "[Robot]";
    }
    else
    {
        $return = "Unknown";
    }

    return $return;
}


/*
* add stats to database
*/
function GUS_addStats( $page, $user_id, $user_name, $host_name, $user_agent )
{
    global $_TABLES, $_GUS_VARS, $QUERY_STRING;

	$sql = "SELECT ua_id FROM {$_TABLES['gus_user_agents']} WHERE user_agent = '{$user_agent}' LIMIT 1";
	$res = DB_query( $sql );

	if ( $row = DB_fetchArray( $res ) )
	{
		$ua_id = $row[0];
	}
	else
	{
		$browser  = GUS_getBrowser( $_SERVER['HTTP_USER_AGENT'] );
		$platform = GUS_getComputerType( $_SERVER['HTTP_USER_AGENT'] );

		$sql = "INSERT INTO {$_TABLES['gus_user_agents']} SET
			user_agent = '{$user_agent}',
			browser = '" . $browser['type'] . "',
			version = '" . $browser['ver'] . "',
			platform = '" . $platform . "'";

		$res = DB_query( $sql,1 );

		$ua_id = DB_insertId();
	}

	// SPECIAL CASE: 404s fill in the query string witht the URI the user was trying to access
	if ( preg_match( "/^.+404\.php$/", $_SERVER['SCRIPT_NAME'] ) )
	{
		$query_string = $_SERVER['REQUEST_URI'];
	}
	else
	{
		$query_string = $_SERVER['QUERY_STRING'];

		// This fixes a problem reported by Tom whereby $_SERVER['QUERY_STRING'] and $QUERY_STRING are not
		//	the same [as they should be...]
		if ( $query_string == '' )
			$query_string = $QUERY_STRING;
	}

	if ( $query_string != '' )
		$query_string = urlencode( DB_escapeString( $query_string ) );

	$referrer = '';

	if ( isset( $_SERVER['HTTP_REFERER'] ) && ( $_SERVER['HTTP_REFERER'] != '' ) )
		$referrer = urlencode( DB_escapeString( $_SERVER['HTTP_REFERER'] ) );

	$sql = "INSERT INTO " . $_TABLES['gus_userstats'] . " SET"
		. " uid='" . $user_id
		. "', username='" . $user_name
		. "', ip='" . $_GUS_VARS['remote_ip']
		. "', host='" . $host_name
		. "', date='" . date( "Y-m-d" )
		. "', time='" . date( "G:i:s" )
		. "', referer='" . $referrer
		. "', query_string='" . $query_string
		. "', request='" . $_SERVER['REQUEST_METHOD']
		. "', page='" . $page
		. "', ua_id='" . $ua_id . "'";

   DB_query( $sql,1 );
}

/**
* Caching Functions
*/

/*
* Function to return cache file name for current page
*/
function GUS_cachefile()
{
    global $_CONF;

    $cfile = basename( $_SERVER['SCRIPT_NAME'] ) . '?' . $_SERVER['QUERY_STRING'];
    $cfile = str_replace('?','_',$cfile);
    $cfile = str_replace('&','_',$cfile);
    $cfile = str_replace('.','_',$cfile);
    $cfile = str_replace('=','_',$cfile);
    $cfile = str_replace('/','_',$cfile);
    $cfile = $_CONF['path_html'] . 'gus/cache/' . $cfile . '.html';
    return $cfile;
}

/*
* Function to return current cache file in string variable
*/

function GUS_getcache()
{
    $buffer = '';
    if ($fd = fopen(GUS_cachefile(),"r")) {
        while (!feof($fd)) {
            $buffer .= fgets($fd,1024);
        }
        fclose($fd);
    } else {
        COM_errorlog( "GUS Cache File " . GUS_cachefile() . " not found", 1 );
    }
    return $buffer;
}

/*
* Function to write string to cache file
*/
function GUS_writecache($buf)
{
    if ($fd = fopen(GUS_cachefile(),"w")) {
        fputs($fd,'<!-- begin cache file -->');
        fputs($fd,$buf);
        fclose($fd);
    } else {
        COM_errorlog( "Unable to open GUS Cache file " . GUS_cachefile() . " for writing", 1 );
    }
}


function GUS_log_and_execute( $sql )
{
    COM_errorLog( $sql, 1 );
    return DB_query( $sql, 1 );
}

/*
* Stats block to show whos online now, # visitors today, registered today, new today, referers today
*
* @return   string  formatted html for block
*/
function phpblock_gusstats()
{
    global $_USER, $_CONF, $_TABLES, $LANG01, $LANG_GUS_wo, $_GUS_CONF;


    if ( !isset($_GUS_CONF['whatsnew_cache_time']) ) {
        $_GUS_CONF['whatsnew_cache_time'] = 90;
    }

    $cacheInstance = 'gusblock__' . $_CONF['theme'];
    $retval = CACHE_check_instance($cacheInstance, 0);
    if ( $retval ) {
        $lu = CACHE_get_instance_update($cacheInstance, 0);
        $now = time();
        if (( $now - $lu ) < $_GUS_CONF['whatsnew_cache_time'] ) {
            return $retval;
        }
    }

    // Code adapted from whose online block
	$expire_time = time() - $_CONF['whosonline_threshold'];
    if ( $_GUS_CONF['wo_fullname'] )
        $byname = 'fullname, username';
    else
        $byname = 'username';

    $num_anon = 0;
    $num_reg  = 0;
	$retval = '';

    if ( $_GUS_CONF['wo_online'] )
	{
        $result = DB_query( "SELECT DISTINCT {$_TABLES['sessions']}.uid, {$byname}, photo, showonline
        		FROM {$_TABLES['sessions']},{$_TABLES['users']},{$_TABLES['userprefs']}
        		WHERE {$_TABLES['sessions']}.uid <> 1
        			AND {$_TABLES['users']}.uid = {$_TABLES['sessions']}.uid
        			AND {$_TABLES['users']}.uid = {$_TABLES['userprefs']}.uid
        			AND start_time >= $expire_time
        		ORDER BY {$byname}" );

		while ( $row = DB_fetchArray( $result, false ) )
        {
            if ( $row['showonline'] == 1 )
            {
                if ( $_GUS_CONF['wo_fullname'] && !empty( $row['fullname'] ) )
                    $username = $row['fullname'];
                else
                    $username = $row['username'];

				$retval .= '<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $row['uid'] . '">'
							. $username . '</a>';

                if ( ($_CONF['allow_user_photo'] == 1) && !empty( $row['photo'] ) )
                {
                    $retval .= '&nbsp;<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $row['uid'] . '">';
                    $retval .= '<img src="' . $_CONF['site_url'] . '/gus/images/user_photo.gif" width="16" height="12" border="0" alt=""/></a>';
                }

                $retval .= '<br/>';
				$num_reg++;
            }
            else
            {
				// this user does not want to show up in Who's Online
                $num_anon++;
            }
        }

		if ( $_GUS_CONF['wo_users_anonymous'] && ( empty( $_USER['uid'] ) || ( $_USER['uid'] == 1 ) ) )
		{
			// note that we're overwriting the contents of $retval here
			if( $num_reg > 0 )
				$retval = $LANG01[112] . ': ' . $num_reg . '<br/>';
			else
				$retval = '';
		}

        $result = DB_query( "SELECT DISTINCT uid, remote_ip
        						FROM {$_TABLES['sessions']}
        						WHERE uid = 1" );
    }
	else
	{
        $result = DB_query( "SELECT DISTINCT uid, remote_ip
        						FROM {$_TABLES['sessions']}" );
    }

    $num_anon += DB_numRows( $result );

	$bot_text = '';

	if ( $_GUS_CONF['wo_show_bots'] )
	{
		$bot_list = array();

		while ( $row = DB_fetchArray( $result, false ) )
		{
			$ip = $row['remote_ip'];

			if ( eregi("^216\.239\.46|^64\.68\.8|^66\.249\.(6[4-9]|[78]\d?|9[0-5])", $ip ) )
			{
				$bot_list[] = "Google";
				$num_anon--;
			}
			else if ( eregi("^68\.142\.(19[2-9]|2[01234]\d?|25[012345])|
						^66\.94\.(22[4-9]|2[3-4]\d?|25[0-5])|
						^216\.109\.(11[2-9]|12[0-7])|
						^206\.190\.(3[2-9]|[4-5]\d?|6[0-3])", $ip ) )
			{
				$bot_list[] = "Yahoo!";
				$num_anon--;
			}
			else if ( eregi("^65\.(19[2-9]|2[0-1]\d?|22[0-3])", $ip ) )
			{
				$bot_list[] = "Ask Jeeves";
				$num_anon--;
			}
			else if ( eregi("^72\.30\.|
						^68\.142\.(19[2-9]|2[0-5]\d?)|
						^66\.196\.(6[4-9]|[789]\d?|1[0-1]\d?|12[0-7])", $ip ) )
			{
				$bot_list[] = "Inktomi";
				$num_anon--;
			}
			else if ( eregi("^64.4.8.9[3-4]|^64.4.8.11[3-7]|^65.5[2-5]\.", $ip ) )
			{
				$bot_list[] = "MSN Bot";
				$num_anon--;
			}
		}

		if ( count( $bot_list ) )
		{
			$bot_text .= "<table style=\"width:100%;\"><tr><td style=\"width:20%;\"><hr/></td>
				<td style=\"white-space:nowrap;\">{$LANG_GUS_wo['bots']}</td>
				<td style=\"width:60%;\"><hr/></td></tr></table>";

			$bots = array_unique( $bot_list );

			sort( $bots );

			foreach( $bots as $bot )
			{
				$bot_text .= $bot . '<br/>';
			}
		}
	}

	if ( $num_anon > 0 )
    {
        if ( $_GUS_CONF['wo_online'] )
            $retval .= $LANG01[41];
		else
            $retval .= $LANG01[17];

        $retval .= ': ' . $num_anon . '<br/>';
    }

	$retval .= $bot_text;

    if ( $_GUS_CONF['wo_daily'] )
	{
        $retval .= "<table style=\"width:100%;\"><tr><td style=\"width:20%;\"><hr/></td>
        			<td style=\"white-space:nowrap;\">{$LANG_GUS_wo['stats']}</td>
       				<td style=\"width:60%;\"><hr/></td></tr></table>";

        $result = DB_query( "SELECT COUNT(*) AS pages FROM {$_TABLES['gus_userstats']} WHERE date = CURRENT_DATE()" );
        $row = DB_fetchArray( $result, false );
        $retval .= $row['pages'] . ' ' . $LANG_GUS_wo['page_title'] . '<br/>';

        $result = DB_query( "SELECT COUNT( DISTINCT ip ) AS visitors FROM {$_TABLES['gus_userstats']} WHERE date = CURRENT_DATE()" );
        $row = DB_fetchArray( $result, false );
        $retval .= $row['visitors'] . ' ' . $LANG_GUS_wo['unique_visitors'] . '<br/>';
    }

    if ( $_GUS_CONF['wo_registered'] )
	{
		$result = DB_query( "SELECT DISTINCT uid FROM {$_TABLES['gus_userstats']} WHERE uid > 1 AND date = CURRENT_DATE()" );

		$num_reg = DB_numRows( $result );

		if ( $num_reg > 0 )
		{
			$retval .= "<table style=\"width:100%;\"><tr><td style=\"width:20%;\"><hr/></td>
						<td style=\"white-space:nowrap;\">{$LANG_GUS_wo['reg_users']}</td>
						<td style=\"width:60%;\"><hr/></td></tr></table>";

			if ( $_GUS_CONF['wo_users_anonymous'] && ( empty( $_USER['uid'] ) || ( $_USER['uid'] == 1 ) ) )
			{
				$retval .= $LANG01[112] . ': ' . $num_reg . '<br/>';
			}
			else
			{
				while ( $row = DB_fetchArray( $result, false ) )
				{
					$rec = DB_query( "SELECT uid, username, fullname, photo
								FROM {$_TABLES['users']}
								WHERE uid = '" . $row['uid'] . "' LIMIT 1" );

					$A = DB_fetchArray( $rec, false );

					$retval .= '<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $A['uid'] . '">';

					if ( $_GUS_CONF['wo_fullname'] && !empty( $A['fullname'] ) )
						$retval .= $A['fullname'];
					else
						$retval .= $A['username'];

					$retval .= '</a>';

					if ( ($_CONF['allow_user_photo'] == 1) && !empty( $A['photo'] ) )
					{
						$retval .= '&nbsp;<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $A['uid'] . '">';
						$retval .= '<img src="' . $_CONF['site_url'] . '/gus/images/user_photo.gif" width="16" height="12" border="0" alt=""/></a>';
					}

					$retval .= '<br/>';
				}
			}
		}
    }

    if ( $_GUS_CONF['wo_new'] )
	{
		$result = DB_query( "SELECT uid, username, fullname, photo FROM {$_TABLES['users']}
								WHERE regdate >= DATE_SUB( CURRENT_DATE(), INTERVAL 1 DAY )" );

		$num_new = DB_numRows( $result );

		if ( $num_new > 0 )
		{
			$retval .= "<table style=\"width:100%;\"><tr><td style=\"width:20%;\"><hr/></td>
						<td style=\"white-space:nowrap;\">{$LANG_GUS_wo['new_users']}</td>
						<td style=\"width:60%;\"><hr/></td></tr></table>";

			if ( $_GUS_CONF['wo_users_anonymous'] && ( empty( $_USER['uid'] ) || ( $_USER['uid'] == 1 ) ) )
			{
				$retval .= $LANG_GUS_wo['new_users'] . ': ' . $num_new . '<br/>';
			}
			else
			{
				while ( $row = DB_fetchArray( $result, false ) )
				{
					$retval .= '<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $row['uid'] . '">';

					if ( $_GUS_CONF['wo_fullname'] && !empty( $row['fullname'] ) )
						$retval .= $row['fullname'];
					else
						$retval .= $row['username'];

					$retval .= '</a>';

					if ( ($_CONF['allow_user_photo'] == 1) && !empty( $row['photo'] ) )
					{
						$retval .= '&nbsp;<a href="' . $_CONF['site_url'] . '/users.php?mode=profile&amp;uid=' . $row['uid'] . '">';
						$retval .= '<img src="' . $_CONF['site_url'] . '/gus/images/user_photo.gif" width="16" height="12" border="0" alt=""/></a>';
					}

					$retval .= '<br/>';
				}
			}
		}
    }

    if ( $_GUS_CONF['wo_refs'] )
	{
		$likeClause = '';

		if ( count( $_GUS_CONF['wo_hide_referrers'] ) )
		{
            foreach ( $_GUS_CONF['wo_hide_referrers'] as $referrer )
			{
				$likeClause .= 'AND referer NOT LIKE \'' . GUS_getEncodedURL( $referrer ) . '%\' ';
            }
		}

		$result = DB_query( "SELECT referer FROM {$_TABLES['gus_userstats']}
							WHERE referer <> ''
								AND date = CURRENT_DATE()
								$likeClause" );

		$header_output = false;

		while ( $row = DB_fetchArray( $result, false ) )
		{
			if ( !$header_output )
			{
				$retval .= "<table style=\"width:100%;\"><tr><td width=\"20%\"><hr/></td>
					<td style=\"white-space:nowrap;\">{$LANG_GUS_wo['referers']}</td>
					<td style=\"width:60%;\"><hr/></td></tr></table>";

				$header_output = true;
			}

			$referrer = urldecode( $row['referer'] );
            $url = parse_url( $referrer );

            if ( $url['host'] != '' )
            {
				$host = preg_replace( '/^www\./', '', $url['host'], 1 );

				if ( $host != '' && $host != $_GUS_VARS['parsed_site_url']['host'] )
				{
					$ref["$host"] += 1;
					$refurl["$host"] = htmlentities( preg_replace( '/admin\/plugins.*$/', '', $referrer, 1 ) );
				}
            }
        }

        if ( is_array($ref) )
		{
            arsort( $ref );
            $i = 0;

            foreach ( $ref as $key => $value )
			{
                $retval .= '<a href="' . $refurl["$key"] . '" rel="nofollow">' . $key . '</a> - ' . $value . '<br/>';

				$i++;

                if ( $i >= $_GUS_CONF['wo_max_referrers'] )
                	break;
            }
        }
    }
    CACHE_create_instance($cacheInstance, $retval, 0);
    return $retval;
}

/*==================================================\
| This does the URLS for page navigation and header |
\==================================================*/

function GUS_create_url( $ignore )
{
	global $_CONF;

	$firstvar = true;
	$qstring = '';

	foreach($_GET as $key => $var)
	{
		if ( ($key != $ignore) && ($key != 'page') ) // always ignore page
		{
			if ($firstvar) // first time through
			{
				$qstring .= "?" . $key . "=" . $var;
				$firstvar = false;
			}
			else
			{
				$qstring .= "&" . $key . "=" . $var;
			}
		}
	}

	$newurl = parse_url( $_CONF['site_url'] );

	$retval = $newurl['scheme'] . "://" . $newurl['host'] . $_SERVER['SCRIPT_NAME'] . $qstring;

	return htmlentities( $retval );
}
?>
