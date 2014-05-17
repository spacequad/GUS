<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | platform.php                                                             |
// | Displays platform specific stats                                         |
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

if ( ($day == 0) && ($month == 0) && ($year == 0) ) {
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
    $T->set_var('data',$LANG_GUS00['page_views']);
    $T->parse('CBlock','COLUMN',false);
    $T->set_var( 'colclass', 'col_left' );

    $T->set_var('data',$LANG_GUS00['platform']);
    $T->parse('CBlock','COLUMN',true);

    $T->set_var( 'rowclass', 'header' );
    $T->parse('BBlock','ROW',true);

    $date_compare = GUS_get_date_comparison( 'date', $year, $month );
    $sql = "SELECT COUNT( platform ) AS count, platform
    	FROM {$_TABLES['gus_userstats']}, {$_TABLES['gus_user_agents']}
    	WHERE {$_TABLES['gus_userstats']}.ua_id = {$_TABLES['gus_user_agents']}.ua_id AND {$date_compare}
    	GROUP BY platform ORDER BY count DESC";

    $rec=DB_query($sql);
    $nrows=DB_numRows($rec);
    for ($i=0; $i<$nrows; $i++)
    {
        if ( ($i + 1) % 2 )
    	    $T->set_var( 'rowclass', 'row1' );
    	else
    	    $T->set_var( 'rowclass', 'row2' );

        $A = DB_fetchArray($rec);

    	$T->set_var( 'colclass', 'col_right' );
        $T->set_var('data',$A[0]);
        $T->parse('CBlock','COLUMN',false);
    	$T->set_var( 'colclass', 'col_left' );

        $T->set_var('data', $A[1]);
        $T->parse('CBlock','COLUMN',true);

        $T->parse('BBlock','ROW',true);
    }

    $T->Parse('ABlock','TABLE',true);

    $title = Date( 'F Y - ', mktime( 0, 0, 0, $month, 1, $year ) ) . $LANG_GUS00['platforms'];

    $display = GUS_template_finish( $T, $title );

    if (($_GUS_cache) && ($today != $month . $year)) {
        GUS_writecache($display);
    }
}

echo COM_siteHeader( $_GUS_CONF['show_left_blocks'] );
echo $display;
echo COM_siteFooter( $_GUS_CONF['show_right_blocks'] );
?>