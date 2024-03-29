<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | security.inc                                                             |
// +--------------------------------------------------------------------------+
// | $Id::                                                                   $|
// +--------------------------------------------------------------------------+
// | Copyright (C) 2009-2011 by the following authors:                        |
// |                                                                          |
// | Mark R. Evans          mark AT glfusion DOT org                          |
// |                                                                          |
// | Based on the GUS Plugin for Geeklog CMS                                  |
// | Copyright (C) 2005 by the following authors:                             |
// |                                                                          |
// | Authors: Andy Maloney      - asmaloney@users.sf.net                      |
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

require_once '../lib-common.php';

if (!in_array('gus', $_PLUGINS)) {
    COM_404();
    exit;
}

function GUS_HasAccess( $allow_anon = true )
{
	global $_CONF, $_GUS_anon_access, $LANG_GUS00, $_USER;

    if ( COM_isAnonUser() && ( $_GUS_anon_access != 1 || $_CONF['loginrequired'] == 1) ) {
        $display = COM_siteHeader();
        $display .= SEC_loginRequiredForm();
        $display .= COM_siteFooter();
        echo $display;
        exit;
    }

	if ( ($allow_anon && $_GUS_anon_access == 1) || SEC_hasRights( 'gus.view' ) )
		return( true );

	COM_errorLog( "Someone has tried to illegally access a GUS stats page.  "
		. "User id: {$_USER['uid']}, Username: {$_USER['username']}, IP: {$_SERVER['REMOTE_ADDR']}", 1 );

	echo COM_siteHeader();
	echo COM_startBlock( $LANG_GUS00['access_denied'] );
	echo $LANG_GUS00['access_denied_msg'];
	echo COM_endBlock();
	echo COM_siteFooter( true );

	return( false );
}

?>