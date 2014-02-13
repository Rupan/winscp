﻿using System.Runtime.InteropServices;
using System;
using System.Globalization;

namespace WinSCP
{
    [Guid("0ADAAEBC-4A15-4A9C-8ED4-D85F5630035C")]
    [ComVisible(true)]
    public enum TransferResumeSupportState
    {
        Default,
        On,
        Off,
        Smart
    }

    [Guid("6CED4579-0DF2-4E46-93E9-18780546B421")]
    [ClassInterface(Constants.ClassInterface)]
    [ComVisible(true)]
    public sealed class TransferResumeSupport
    {
        public TransferResumeSupportState State { get; set; }
        public int Threshold { get { return GetThreshold(); } set { SetThreshold(value); } }

        internal TransferResumeSupport()
        {
            State = TransferResumeSupportState.Default;
            _threshold = 100; // (100 KiB)
        }

        public override string ToString()
        {
            string result;
            switch (State)
            {
                case TransferResumeSupportState.Default:
                    result = "default";
                    break;
                case TransferResumeSupportState.Off:
                    result = "off";
                    break;
                case TransferResumeSupportState.On:
                    result = "on";
                    break;
                case TransferResumeSupportState.Smart:
                    result = Threshold.ToString(CultureInfo.InvariantCulture);
                    break;
                default:
                    result = "unknown";
                    break;
            }
            return result;
        }

        private int GetThreshold()
        {
            CheckSmart();
            return _threshold;
        }

        private void SetThreshold(int threshold)
        {
            if (threshold <= 0)
            {
                throw new ArgumentOutOfRangeException("threshold", "Threshold must be positive");
            }
            CheckSmart();
            _threshold = threshold;
        }

        private void CheckSmart()
        {
            if (State != TransferResumeSupportState.Smart)
            {
                throw new InvalidOperationException("Threshold is undefined when state is not Smart");
            }
        }

        private int _threshold;
    }
}
