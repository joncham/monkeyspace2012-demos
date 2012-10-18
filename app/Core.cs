using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;

namespace EmbedSample
{
	public struct OperationData
	{
		public IntPtr Name;
		public IntPtr GCHandle;
	}

	public struct ArrayHelper
	{
		public int Length;
		public IntPtr Data;
	}

	public delegate ArrayHelper GetOperationsFunc([MarshalAs(UnmanagedType.LPWStr)]string p);
	public delegate double ExecuteFunc(IntPtr handle, double a, double b);

	public struct EmbedAPI
	{
		public GetOperationsFunc GetOperations;
		public ExecuteFunc Execute;
	}

	public class EmbedHelper
	{
		static EmbedAPI _api;

		static IntPtr GetAPI()
		{
			_api = new EmbedAPI() {
				GetOperations = GetOperations,
				Execute = ExecuteHelper
			};
			IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(_api));
			Marshal.StructureToPtr(_api, ptr, false);
			return ptr;
		}

		public  static int DotNetEntryPoint(String pwzArgument)
		{
			long l = long.Parse(pwzArgument);
			IntPtr ptr = (IntPtr)l;
			Marshal.WriteIntPtr(ptr, GetAPI());

			return 1;
		}

		static int Main(string[] args)
		{
			return 0;
		}

		static ArrayHelper GetOperations(string path)
		{
			var arrayHelper = new ArrayHelper();
			try
			{
				List<string> files = new List<string>(Directory.GetFiles(path, "*.dll"));
				List<OperationData> operations = new List<OperationData>();
				foreach (string file in files)
				{
					if (file.EndsWith("Core.dll"))
						continue;

					operations.Add(CreateOperation(file));
				}
				arrayHelper.Length = operations.Count;
				arrayHelper.Data = MarshalOperations(operations);
				
			}
			catch (Exception ex)
			{
				File.WriteAllText("error.txt", "ex " + ex);
			}
			return arrayHelper;
		}

		static IntPtr MarshalOperations(List<OperationData> operations)
		{
			int sizeOfOperationData = Marshal.SizeOf(typeof(OperationData));
			IntPtr ptr;
			IntPtr ptr_orig = ptr = Marshal.AllocHGlobal(sizeOfOperationData * operations.Count);

			foreach (var operation in operations)
			{
				Marshal.StructureToPtr(operation, ptr, false);
				ptr = (IntPtr)(ptr.ToInt64() + sizeOfOperationData);
			}

			return ptr_orig;
		}

		static double ExecuteHelper(IntPtr handle, double a, double b)
		{
			try
			{
				object target = ((GCHandle)handle).Target;
				IOperation operation = (IOperation)target;
				return operation.Execute(a, b);
			}
			catch (Exception ex)
			{
				File.WriteAllText("error.txt", "ex " + ex);
			}

			return 0.0;
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
					return new OperationData() { Name = Marshal.StringToHGlobalUni(o.Name), GCHandle = handle };
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

	public class SecureMethods
	{
		[SecurityCritical]
		public static void WriteToDisk()
		{
			File.WriteAllText("jon.txt", "I am in your hard drive");
		}
	}
}
