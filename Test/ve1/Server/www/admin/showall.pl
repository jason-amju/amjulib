#!/usr/bin/perl -w

#############################################
## SHOW PLAYERS
#############################################

require "../../cgi-bin/common.pl";

my_connect();

print "<br>PLAYERS<br>\n";
sub showplayers();
showplayers();

print "<br><br>OBJECTS<br>\n";
sub showobjects();
showobjects();

print "That's it!\n";

##### end of main function

sub showplayers()
{
  my $findplayersql = "select * from player";

  print "Query: $findplayersql\n\n";
  if (not $dbh) { print "Oh no, dbh is undefined!?!?!\n"; }

  my $query = $dbh->prepare($findplayersql) or die
    "Query prepare failed for this query: $findplayersql\n";

  $query->execute;
  print "<table border=1>";
  print "<tr><td>ID</td> <td>Name</td> <td>Email</td> <td>Hashpw</td> <td>Thumbnail</td> <td>Last time</td> <td>Object ID</td></tr>\n";
  while (my ($player_id, $name, $email, $hashpw, $thumbnail, $lasttime, $obj_id) = $query->fetchrow_array)
  {
    print "<tr>";
    print "<td>$player_id</td> <td>$name</td> <td>$email</td> <td>$hashpw</td> <td>$thumbnail</td> <td>$lasttime</td> <td>$obj_id</td>\n";
    print "</tr>";
  }
  print "</table>";

}

sub showobjects()
{
  my $sql = "select * from object";

  print "Query: $sql\n\n";
  if (not $dbh) { print "dbh is undefined!?!?!\n"; }

  my $query = $dbh->prepare($sql) or die
    "Query prepare failed for this query: $sql\n";

  $query->execute;

  print "<table border=1>";
  print "<tr><td>ID</td> <td>Type</td> <td>Assetfile</td> <td>Datafile</td> <td>Owner ID</td> <td>Create time</td></tr>\n";

  while (my ($id, $type, $assetfile, $datafile, $owner, $lasttime) = $query->fetchrow_array)
  {
    print "<tr>";
    print "<td>$id</td> <td>$type</td> <td>$assetfile</td> <td>$datafile</td> <td>$owner</td> <td>$lasttime</td>\n";
    print "</tr>";
  }
  print "</table>";

}


