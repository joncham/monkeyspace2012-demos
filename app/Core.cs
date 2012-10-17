using System;

namespace EmbedSample
{
	public interface IOperation
	{
		string Name { get; }
		double Execute(double a, double b);
	}
}
