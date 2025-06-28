/*
 * This example tests a switch with a GNU extension.
 */


static int f (int i)
{
    switch (i) {
    case 0:
    case 1:
        return 0;
    case 2:
        return 7;
    case 3:
    case 4:
    case 5:
        return 1;

    case 12 ... 23:  /* GNU extension */
        return 23;

    default:
        return 13;
    }
}

int main(int argc, char** argv)
{
  int i;
  for (i = -3; i < 27; ++i)
    {
      int j = f (i);
      int k;
      if (i == 0 || i == 1)
	{
	  k = 0;
	}
      else if (i == 2)
	{
	  k = 7;
	}
      else if (i == 3 || i == 4 || i == 5)
	{
	  k = 1;
	}
      else if (i >= 12 && i <= 32)
	{
	  k = 23;
	}
      else
	{
	  k = 13;
	}

      if (k != j)
	{
	  return 1;
	}
    }
  return 0;
}
