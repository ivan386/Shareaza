using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;
using System.Reflection;

namespace Updater.Common
{
	class ImplementedViews
	{
		static IEnumerable<Type> classes;
		
		static ImplementedViews() {
			const string className = @"AbstractGenericPageView";
			
			Assembly asm = Assembly.GetExecutingAssembly();
			classes = from view in asm.GetTypes()
					  let baseType = view.BaseType
					  where view.IsClass && !view.IsAbstract && !view.IsGenericType &&
							baseType != null && baseType.IsAbstract && baseType.IsGenericType &&
							baseType.Name.StartsWith(className)
					  select view;
		}
		
		public static Dictionary<string, Type> GetElementMappings() {
			var info = from FieldInfo fieldInfo in typeof(skin).GetFields(BindingFlags.Public |
																		  BindingFlags.Instance)
					   where fieldInfo.FieldType.IsArray
					   select fieldInfo;

			var valid = from f in info
						let attributes = f.GetCustomAttributes(typeof(XmlArrayItemAttribute), false)
						let elementTypeName = f.FieldType.FullName.TrimEnd(new char[] { '[', ']' })
						let type = Type.GetType(elementTypeName)
						where type.IsClass
						select new { Name = f.Name, Attributes = attributes, Type = type };

			var mappings = from f in valid
						   let attribute = f.Name
						   let mapping = Char.ToUpper(attribute[0]) + attribute.Substring(1)
						   select new { Name = mapping, Type = f.Type };
			return mappings.ToDictionary(m => m.Name, m => m.Type);
		}

		public static IPageView GetView(Type genericType) {
			var instanceTypes = from c in classes
								let gt = c.BaseType.GetGenericArguments()
								where gt.Length == 1 && gt[0].Equals(genericType)
								select c;
			if (!instanceTypes.Any())
				return null;
			try {
				return (IPageView)Activator.CreateInstance(instanceTypes.First());
			} catch { }
			return null;
		}
	}
}
