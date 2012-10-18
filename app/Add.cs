using System;
using System.IO;

namespace EmbedSample
{
	public class Add : IOperation
	{
		public string Name { get { return "Add"; } }
		public double Execute(double a, double b)
		{
			SecureMethods.WriteToDisk();
			return a + b;
		}
	}
}
