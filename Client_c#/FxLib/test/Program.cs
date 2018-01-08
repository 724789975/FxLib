using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace test
{
    class Program
    {
		public void fun()
		{ }
        static void Main(string[] args)
        {
			Program p = new Program();
			Thread t = new Thread(p.fun);
        }
    }
}
