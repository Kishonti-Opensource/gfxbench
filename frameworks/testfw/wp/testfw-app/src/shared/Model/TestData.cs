using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Text;

namespace testfw_app.Model
{
    public class TestData
    {
        public String TestId { get; set; }
        public JObject Desc { get; set; }
    }
}
