<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | links.php                                                                |
// | Displays stats on referers                                               |
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
// |          Danny Ledger      - squatty@users.sourceforge.net               |
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

// check for cached file
if ($day == '' || $day == 0) {
    $today = date("nY");
} else {
    $today = date("jnY");
}

if ( ($day == 0) && ($month == 0) && ($year == 0) ) {
	$year   = date( 'Y' );
	$month  = date( 'n' );
	$day    = date( 'j' );
}

if ((file_exists(GUS_cachefile())) && ($today != $day . $month . $year)) {
    $display = GUS_getcache();
} else {
    // no cached version
    $T = GUS_template_start();

    $T->set_block('page','COLUMN','CBlock');
    $T->set_block('page','ROW','BBlock');
    $T->set_block('page','TABLE','ABlock');

    $T->set_var( 'colclass', 'col_right' );
    $T->set_var('data',$LANG_GUS00['count']);
    $T->parse('CBlock','COLUMN',false);

    $T->set_var( 'colclass', 'col_left' );
    $T->set_var('data',$LANG_GUS00['link']);
    $T->parse('CBlock','COLUMN',true);

    $T->set_var( 'rowclass', 'header' );
    $T->parse('BBlock','ROW',true);

    $date_compare = GUS_get_date_comparison( 'date', $year, $month, $day );

    $sql = "SELECT COUNT( query_string ) AS cnt, query_string FROM {$_TABLES['gus_userstats']} ";

    if ( $_GUS_phplinks == 1 )
    {
        $outer_frame = DB_getItem( $_TABLES['plsettings'], 'OuterFrame',"ID = '1'" );

    	if ( $outer_frame == "N" )
    		$sql .= "WHERE page='phplinks/out.php' ";
    	else
    		$sql .= "WHERE page='phplinks/out_frame.php' ";
    }
    else
    {
    	$sql .= "WHERE page LIKE '%portal.php' AND query_string <> '' ";
    }

    $sql .= "AND {$date_compare} GROUP BY query_string ORDER BY cnt DESC";

    $rec = DB_query($sql);
    $nrows = DB_numRows($rec);

    for ($i=0; $i<$nrows; $i++)
    {
        if ( ($i + 1) % 2 )
    	    $T->set_var( 'rowclass', 'row1' );
    	else
    	    $T->set_var( 'rowclass', 'row2' );

        $A=DB_fetchArray($rec);

        $query_string = urldecode( $A['query_string'] );
        $query1 = explode( '&', $query_string );

        if ($_GUS_phplinks == 1)
        {
            $outer_frame = DB_getItem($_TABLES['plsettings'], 'OuterFrame',"ID = '1'" );
            if ($outer_frame == "N")
            {
    	        $query = explode('=',$query1[1]);
    	        $id = $query[1];
    	        if (!empty($id)){
    	           $sql = "SELECT SiteName FROM {$_TABLES['pllinks']} WHERE id=$id";
    	           $row = DB_fetchArray(DB_query($sql));
    	           $title = $row[SiteName];
    	        }

    			$T->set_var( 'colclass', 'col_right' );
    	        $T->set_var('data',$A['cnt']);
    	        $T->parse('CBlock','COLUMN',false);
    	        $T->set_var('data','<a href="' . $_CONF['site_url'] . '/phplinks/out.php?&ID=' . $id . '">' . $title . "</a>");
    	    }
    	    else
    	    {
    	        $query = explode('=',$query1[1]);
    	        $id = $query[1];
    	        if (!empty($id)){
    	           $sql = "select SiteName from {$_TABLES['pllinks']} where id=$id";
    	           $row = DB_fetchArray(DB_query($sql));
    	           $title = $row[SiteName];
    	        }

    			$T->set_var( 'colclass', 'col_right' );
    	        $T->set_var('data',$A['cnt']);
    	        $T->parse('CBlock','COLUMN',false);
    	        $T->set_var('data','<a href="' . $_CONF['site_url'] . '/phplinks/out_frame.php?&ID=' . $id . '">' . $title . "</a>");
               }
        }
        else
        {
            $query = explode('=',$query1[0]);
            if ($query[0] == 'url') {
                $title = $query[1];
            } elseif ($query[0] == 'what') {
                $query = explode('=',$query1[1]);
                $id = $query[1];
                $title = DB_getItem ($_TABLES['links'], 'url', "lid = '{$id}'");
            }

    		$T->set_var( 'colclass', 'col_right' );
            $T->set_var('data',$A['cnt']);
            $T->parse('CBlock','COLUMN',false);
            $T->set_var('data','<a href="' . $title . '" target="_blank">' . $title . "</a>");
        }

    	$T->set_var( 'colclass', 'col_left' );
        $T->parse('CBlock','COLUMN',true);

        $T->parse('BBlock','ROW',true);
    }

    $T->Parse('ABlock','TABLE',true);

    if ( $day != '' )
        $title = Date( 'l, j F, Y - ', mktime( 0, 0, 0, $month, $day, $year ) ) . $LANG_GUS00['links_followed'];
    else
        $title = Date( 'F Y - ', mktime( 0, 0, 0, $month, 1, $year ) ) . $LANG_GUS00['links_followed'];

    $display = GUS_template_finish( $T, $title );

    if (($_GUS_cache) && ($today != $day . $month . $year)) {
        GUS_writecache($display);
    }
}

echo COM_siteHeader( $_GUS_CONF['show_left_blocks'] );
echo $display;
echo COM_siteFooter( $_GUS_CONF['show_right_blocks'] );
?>