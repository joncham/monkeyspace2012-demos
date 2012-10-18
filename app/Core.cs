using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace EmbedSample
{
	public struct OperationData
	{
		public string Name;
		public IntPtr GCHandle;
	}

	public class EmbedHelper
	{
		static OperationData[] GetOperations(string path)
		{
			var operations = new List<OperationData>();
			foreach (string file in Directory.GetFiles(path, "*.dll"))
			{
				if (file.EndsWith("Core.dll"))
					continue;

				operations.Add(CreateOperation(file));
			}

			return operations.ToArray();
		}

		static OperationData CreateOperation(string path)
		{
			Assembly assembly = Assembly.LoadFrom(path);
			foreach (var type in assembly.GetExportedTypes())
			{
				if (typeof(IOperation).IsAssignableFrom(type))
				{
					IOperation o = (IOperation)Activator.CreateInstance(type);
					IntPtr handle = (IntPtr)GCHandle.Alloc(o);
					return new OperationData() { Name = o.Name, GCHandle = handle };
				}
			}

			return new OperationData();
		}
	}

	public interface IOperation
	{
		string Name { get; }
		double Execute(double a, double b);
	}
}
