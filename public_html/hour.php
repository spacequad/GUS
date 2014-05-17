<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | hour.php                                                                 |
// | Displays stats for a specific hour                                       |
// +--------------------------------------------------------------------------+
// | $Id::                                                                   $|
// +--------------------------------------------------------------------------+
// | Copyright (C) 2008-2011 by the following authors:                        |
// |                                                                          |
// | Mark R. Evans          mark AT glfusion DOT org                          |
// |                                                                          |
// | Based on the GUS Plugin for Geeklog                                      |
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

require_once './include/security.inc';

if ( !GUS_HasAccess() )
	exit;

require_once './include/sql.inc';
require_once './include/util.inc';

/*
* Main Function
*/

if ( ($month == 0) && ($year == 0) ) {
	$year   = date( 'Y' );
	$month  = date( 'n' );
}

// check for cached file
$today = date("nY");
if ((file_exists(GUS_cachefile())) && ($today != $month . $year)) {
    $display = GUS_getcache();
} else {
    // no cached version

    $T = GUS_template_start();

    $T->set_block('page','COLUMN','CBlock');
    $T->set_block('page','ROW','BBlock');
    $T->set_block('page','TABLE','ABlock');

    $T->set_var( 'colclass', 'col_right' );

    $T->set_var('data',$LANG_GUS00['hour']);
    $T->parse('CBlock','COLUMN',false);

    $T->set_var('data',$LANG_GUS00['anon_users']);
    $T->parse('CBlock','COLUMN',true);

    $T->set_var('data',$LANG_GUS00['reg_users']);
    $T->parse('CBlock','COLUMN',true);

    $T->set_var('data',$LANG_GUS00['pages']);
    $T->parse('CBlock','COLUMN',true);

    $T->set_var( 'rowclass', 'header' );
    $T->parse('BBlock','ROW',true);

    $temp_table = GUS_create_temp_userstats_table( $year, $month );

    for ( $i=0; $i < 24; $i++ )
    {
        if ( ($i + 1) % 2 )
    	    $T->set_var( 'rowclass', 'row1' );
    	else
    	    $T->set_var( 'rowclass', 'row2' );

    	$T->set_var( 'data', $i );
        $T->parse( 'CBlock', 'COLUMN', false );

        $result = DB_query( "SELECT COUNT( DISTINCT ip ) AS num_anon FROM {$temp_table['name']} WHERE uid = '1' AND HOUR( time ) = $i" );
    	$row = DB_fetchArray( $result, false );
    	$T->set_var( 'data', $row['num_anon'] );
        $T->parse( 'CBlock', 'COLUMN', true );

        $result = DB_query( "SELECT COUNT( DISTINCT ip ) AS num_registered FROM {$temp_table['name']} WHERE uid > '1' AND HOUR( time ) = $i" );
    	$row = DB_fetchArray( $result, false );
    	$T->set_var( 'data', $row['num_registered'] );
        $T->parse( 'CBlock', 'COLUMN', true );

        $result = DB_query( "SELECT COUNT(*) AS num_pages FROM {$temp_table['name']} WHERE HOUR( time ) = $i" );
    	$row = DB_fetchArray( $result, false );
    	$T->set_var( 'data', $row['num_pages'] );
        $T->parse( 'CBlock', 'COLUMN', true );

        $T->parse( 'BBlock', 'ROW', true );
    }
    $T->Parse( 'ABlock', 'TABLE', true );

    $title = Date( 'F Y - ', mktime( 0, 0, 0, $month, 1, $year ) ) . $LANG_GUS00['views_per_hour'];

    $display = GUS_template_finish( $T, $title );

    if (($_GUS_cache) && ($today != $month . $year)) {
        GUS_writecache($display);
    }

    GUS_remove_temp_table( $temp_table );
}

echo COM_siteHeader( $_GUS_CONF['show_left_blocks'] );
echo $display;
echo COM_siteFooter( $_GUS_CONF['show_right_blocks'] );
?>