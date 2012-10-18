using System;

namespace EmbedSample
{
	public class Divide : IOperation
	{
		public string Name { get { return "Divide"; } }
		public double Execute(double a, double b)
		{
			return a / b;
		}
	}
}
