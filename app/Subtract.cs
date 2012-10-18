using System;

namespace EmbedSample
{
	public class Subtract : IOperation
	{
		public string Name { get { return "Subtract"; } }
		public double Execute(double a, double b)
		{
			return a - b;
		}
	}
}
