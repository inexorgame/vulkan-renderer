/*
Used in GitHub Actions to validate commits.
 */
let Commit
let CommitValidator
let Result
let Status

export function import_types(commitValidatorCls, commitCls, resultCls, statusCls) {
    CommitValidator = commitValidatorCls
    Commit = commitCls
    Result = resultCls
    Status = statusCls
}

export function createValidator() {
    return class Validator extends CommitValidator {
        static rx_parser = new RegExp('^\\[(.*)] (.*)$')
        static rx_category = new RegExp('^\\*|(?:[a-z0-9]{2,}[ |-]?)+$')
        static rx_description = new RegExp('^[A-Z0-9]\\S*(?:\\s\\S*)+[^.!?,\\s]$')

        validate_message(summary, _description) {
            const match = Validator.rx_parser.exec(summary)
            if (match === null) {
                return new Result(
                    Status.Failure,
                    'Summary has invalid format. It should be \'[<tag>] <Good Description>\''
                )
            }
            if (!Validator.rx_category.test(match[1])) {
                return new Result(
                    Status.Failure,
                    "Invalid category tag. It should be either a single '*' or completely lowercase " +
                    "letters or numbers, at least 2 characters long, other allowed characters are: '|', '-' and spaces."
                )
            }
            if (!Validator.rx_description.test(match[2])) {
                return new Result(
                    Status.Failure,
                    'Invalid description. It should start with an uppercase letter or number, ' +
                    'should be not to short and should not end with a punctuation.'
                )
            }
            return new Result(Status.Ok)
        }
    }
}
